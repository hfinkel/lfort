//===--- ExecuteCompilerInvocation.cpp ------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file holds ExecuteCompilerInvocation(). It is split into its own file to
// minimize the impact of pulling in essentially everything else in LFort.
//
//===----------------------------------------------------------------------===//

#include "lfort/FrontendTool/Utils.h"
#include "lfort/CodeGen/CodeGenAction.h"
#include "lfort/Driver/OptTable.h"
#include "lfort/Driver/Option.h"
#include "lfort/Driver/Options.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/CompilerInvocation.h"
#include "lfort/Frontend/FrontendActions.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/Frontend/FrontendPluginRegistry.h"
#include "lfort/Rewrite/Frontend/FrontendActions.h"
#include "lfort/StaticAnalyzer/Frontend/FrontendActions.h"
#include "llvm/Support/DynamicLibrary.h"
#include "llvm/Support/ErrorHandling.h"
using namespace lfort;

static FrontendAction *CreateFrontendBaseAction(CompilerInstance &CI) {
  using namespace lfort::frontend;
  StringRef Action("unknown");

  switch (CI.getFrontendOpts().ProgramAction) {
  case ASTDeclList:            return new ASTDeclListAction();
  case ASTDump:                return new ASTDumpAction();
  case ASTDumpXML:             return new ASTDumpXMLAction();
  case ASTPrint:               return new ASTPrintAction();
  case ASTView:                return new ASTViewAction();
  case DumpRawTokens:          return new DumpRawTokensAction();
  case DumpTokens:             return new DumpTokensAction();
  case EmitAssembly:           return new EmitAssemblyAction();
  case EmitBC:                 return new EmitBCAction();
#ifdef LFORT_ENABLE_REWRITER
  case EmitHTML:               return new HTMLPrintAction();
#else
  case EmitHTML:               Action = "EmitHTML"; break;
#endif
  case EmitLLVM:               return new EmitLLVMAction();
  case EmitLLVMOnly:           return new EmitLLVMOnlyAction();
  case EmitCodeGenOnly:        return new EmitCodeGenOnlyAction();
  case EmitObj:                return new EmitObjAction();
#ifdef LFORT_ENABLE_REWRITER
  case FixIt:                  return new FixItAction();
#else
  case FixIt:                  Action = "FixIt"; break;
#endif
  case GeneratePCModule:         return new GeneratePCModuleAction;
  case GeneratePCH:            return new GeneratePCHAction;
  case GeneratePTH:            return new GeneratePTHAction();
  case InitOnly:               return new InitOnlyAction();
  case ParseSyntaxOnly:        return new SyntaxOnlyAction();

  case PluginAction: {
    for (FrontendPluginRegistry::iterator it =
           FrontendPluginRegistry::begin(), ie = FrontendPluginRegistry::end();
         it != ie; ++it) {
      if (it->getName() == CI.getFrontendOpts().ActionName) {
        OwningPtr<PluginASTAction> P(it->instantiate());
        if (!P->ParseArgs(CI, CI.getFrontendOpts().PluginArgs))
          return 0;
        return P.take();
      }
    }

    CI.getDiagnostics().Report(diag::err_fe_invalid_plugin_name)
      << CI.getFrontendOpts().ActionName;
    return 0;
  }

  case PrintDeclContext:       return new DeclContextPrintAction();
  case PrintPreamble:          return new PrintPreambleAction();
  case PrintPreprocessedInput: {
    if (CI.getPreprocessorOutputOpts().RewriteIncludes) {
#ifdef LFORT_ENABLE_REWRITER
      return new RewriteIncludesAction();
#else
      Action = "RewriteIncludesAction";
      break;
#endif
    }
    return new PrintPreprocessedAction();
  }

#ifdef LFORT_ENABLE_REWRITER
  case RewriteMacros:          return new RewriteMacrosAction();
  case RewriteObjC:            return new RewriteObjCAction();
  case RewriteTest:            return new RewriteTestAction();
#else
  case RewriteMacros:          Action = "RewriteMacros"; break;
  case RewriteObjC:            Action = "RewriteObjC"; break;
  case RewriteTest:            Action = "RewriteTest"; break;
#endif
#ifdef LFORT_ENABLE_ARCMT
  case MigrateSource:          return new arcmt::MigrateSourceAction();
#else
  case MigrateSource:          Action = "MigrateSource"; break;
#endif
#ifdef LFORT_ENABLE_STATIC_ANALYZER
  case RunAnalysis:            return new ento::AnalysisAction();
#else
  case RunAnalysis:            Action = "RunAnalysis"; break;
#endif
  case RunPreprocessorOnly:    return new PreprocessOnlyAction();
  }

#if !defined(LFORT_ENABLE_ARCMT) || !defined(LFORT_ENABLE_STATIC_ANALYZER) \
  || !defined(LFORT_ENABLE_REWRITER)
  CI.getDiagnostics().Report(diag::err_fe_action_not_available) << Action;
  return 0;
#else
  llvm_unreachable("Invalid program action!");
#endif
}

static FrontendAction *CreateFrontendAction(CompilerInstance &CI) {
  // Create the underlying action.
  FrontendAction *Act = CreateFrontendBaseAction(CI);
  if (!Act)
    return 0;

  const FrontendOptions &FEOpts = CI.getFrontendOpts();

#ifdef LFORT_ENABLE_REWRITER
  if (FEOpts.FixAndRecompile) {
    Act = new FixItRecompile(Act);
  }
#endif
  
#ifdef LFORT_ENABLE_ARCMT
  // Potentially wrap the base FE action in an ARC Migrate Tool action.
  switch (FEOpts.ARCMTAction) {
  case FrontendOptions::ARCMT_None:
    break;
  case FrontendOptions::ARCMT_Check:
    Act = new arcmt::CheckAction(Act);
    break;
  case FrontendOptions::ARCMT_Modify:
    Act = new arcmt::ModifyAction(Act);
    break;
  case FrontendOptions::ARCMT_Migrate:
    Act = new arcmt::MigrateAction(Act,
                                   FEOpts.MTMigrateDir,
                                   FEOpts.ARCMTMigrateReportOut,
                                   FEOpts.ARCMTMigrateEmitARCErrors);
    break;
  }

  if (FEOpts.ObjCMTAction != FrontendOptions::ObjCMT_None) {
    Act = new arcmt::ObjCMigrateAction(Act, FEOpts.MTMigrateDir,
                   FEOpts.ObjCMTAction & ~FrontendOptions::ObjCMT_Literals,
                   FEOpts.ObjCMTAction & ~FrontendOptions::ObjCMT_Subscripting);
  }
#endif

  // If there are any AST files to merge, create a frontend action
  // adaptor to perform the merge.
  if (!FEOpts.ASTMergeFiles.empty())
    Act = new ASTMergeAction(Act, FEOpts.ASTMergeFiles);

  return Act;
}

bool lfort::ExecuteCompilerInvocation(CompilerInstance *LFort) {
  // Honor -help.
  if (LFort->getFrontendOpts().ShowHelp) {
    OwningPtr<driver::OptTable> Opts(driver::createDriverOptTable());
    Opts->PrintHelp(llvm::outs(), "lfort -cc1",
                    "LLVM 'LFort' Compiler: http://lfort.llvm.org",
                    /*Include=*/driver::options::CC1Option,
                    /*Exclude=*/0);
    return 0;
  }

  // Honor -version.
  //
  // FIXME: Use a better -version message?
  if (LFort->getFrontendOpts().ShowVersion) {
    llvm::cl::PrintVersionMessage();
    return 0;
  }

  // Load any requested plugins.
  for (unsigned i = 0,
         e = LFort->getFrontendOpts().Plugins.size(); i != e; ++i) {
    const std::string &Path = LFort->getFrontendOpts().Plugins[i];
    std::string Error;
    if (llvm::sys::DynamicLibrary::LoadLibraryPermanently(Path.c_str(), &Error))
      LFort->getDiagnostics().Report(diag::err_fe_unable_to_load_plugin)
        << Path << Error;
  }

  // Honor -mllvm.
  //
  // FIXME: Remove this, one day.
  // This should happen AFTER plugins have been loaded!
  if (!LFort->getFrontendOpts().LLVMArgs.empty()) {
    unsigned NumArgs = LFort->getFrontendOpts().LLVMArgs.size();
    const char **Args = new const char*[NumArgs + 2];
    Args[0] = "lfort (LLVM option parsing)";
    for (unsigned i = 0; i != NumArgs; ++i)
      Args[i + 1] = LFort->getFrontendOpts().LLVMArgs[i].c_str();
    Args[NumArgs + 1] = 0;
    llvm::cl::ParseCommandLineOptions(NumArgs + 1, Args);
  }

#ifdef LFORT_ENABLE_STATIC_ANALYZER
  // Honor -analyzer-checker-help.
  // This should happen AFTER plugins have been loaded!
  if (LFort->getAnalyzerOpts()->ShowCheckerHelp) {
    ento::printCheckerHelp(llvm::outs(), LFort->getFrontendOpts().Plugins);
    return 0;
  }
#endif

  // If there were errors in processing arguments, don't do anything else.
  bool Success = false;
  if (!LFort->getDiagnostics().hasErrorOccurred()) {
    // Create and execute the frontend action.
    OwningPtr<FrontendAction> Act(CreateFrontendAction(*LFort));
    if (Act) {
      Success = LFort->ExecuteAction(*Act);
      if (LFort->getFrontendOpts().DisableFree)
        Act.take();
    }
  }

  return Success;
}
