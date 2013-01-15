//===-- examples/lfort-interpreter/main.cpp - LFort C Interpreter Example -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/CodeGen/CodeGenAction.h"
#include "lfort/Basic/DiagnosticOptions.h"
#include "lfort/Driver/Compilation.h"
#include "lfort/Driver/Driver.h"
#include "lfort/Driver/Tool.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/CompilerInvocation.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/Frontend/TextDiagnosticPrinter.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
using namespace lfort;
using namespace lfort::driver;

// This function isn't referenced outside its translation unit, but it
// can't use the "static" keyword because its address is used for
// GetMainExecutable (since some platforms don't support taking the
// address of main, and some platforms can't implement GetMainExecutable
// without being given the address of a function in the main executable).
llvm::sys::Path GetExecutablePath(const char *Argv0) {
  // This just needs to be some symbol in the binary; C++ doesn't
  // allow taking the address of ::main however.
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  return llvm::sys::Path::GetMainExecutable(Argv0, MainAddr);
}

static int Execute(llvm::Module *Mod, char * const *envp) {
  llvm::InitializeNativeTarget();

  std::string Error;
  OwningPtr<llvm::ExecutionEngine> EE(
    llvm::ExecutionEngine::createJIT(Mod, &Error));
  if (!EE) {
    llvm::errs() << "unable to make execution engine: " << Error << "\n";
    return 255;
  }

  llvm::Function *EntrySubPgm = Mod->getSubprogram("main");
  if (!EntrySubPgm) {
    llvm::errs() << "'main' function not found in module.\n";
    return 255;
  }

  // FIXME: Support passing arguments.
  std::vector<std::string> Args;
  Args.push_back(Mod->getPCModuleIdentifier());

  return EE->runSubprogramAsMain(EntrySubPgm, Args, envp);
}

int main(int argc, const char **argv, char * const *envp) {
  void *MainAddr = (void*) (intptr_t) GetExecutablePath;
  llvm::sys::Path Path = GetExecutablePath(argv[0]);
  IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts = new DiagnosticOptions();
  TextDiagnosticPrinter *DiagClient =
    new TextDiagnosticPrinter(llvm::errs(), &*DiagOpts);

  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  DiagnosticsEngine Diags(DiagID, &*DiagOpts, DiagClient);
  Driver TheDriver(Path.str(), llvm::sys::getDefaultTargetTriple(),
                   "a.out", Diags);
  TheDriver.setTitle("lfort interpreter");

  // FIXME: This is a hack to try to force the driver to do something we can
  // recognize. We need to extend the driver library to support this use model
  // (basically, exactly one input, and the operation mode is hard wired).
  llvm::SmallVector<const char *, 16> Args(argv, argv + argc);
  Args.push_back("-fsyntax-only");
  OwningPtr<Compilation> C(TheDriver.BuildCompilation(Args));
  if (!C)
    return 0;

  // FIXME: This is copied from ASTUnit.cpp; simplify and eliminate.

  // We expect to get back exactly one command job, if we didn't something
  // failed. Extract that job from the compilation.
  const driver::JobList &Jobs = C->getJobs();
  if (Jobs.size() != 1 || !isa<driver::Command>(*Jobs.begin())) {
    SmallString<256> Msg;
    llvm::raw_svector_ostream OS(Msg);
    C->PrintJob(OS, C->getJobs(), "; ", true);
    Diags.Report(diag::err_fe_expected_compiler_job) << OS.str();
    return 1;
  }

  const driver::Command *Cmd = cast<driver::Command>(*Jobs.begin());
  if (llvm::StringRef(Cmd->getCreator().getName()) != "lfort") {
    Diags.Report(diag::err_fe_expected_lfort_command);
    return 1;
  }

  // Initialize a compiler invocation object from the lfort (-cc1) arguments.
  const driver::ArgStringList &CCArgs = Cmd->getArguments();
  OwningPtr<CompilerInvocation> CI(new CompilerInvocation);
  CompilerInvocation::CreateFromArgs(*CI,
                                     const_cast<const char **>(CCArgs.data()),
                                     const_cast<const char **>(CCArgs.data()) +
                                       CCArgs.size(),
                                     Diags);

  // Show the invocation, with -v.
  if (CI->getHeaderSearchOpts().Verbose) {
    llvm::errs() << "lfort invocation:\n";
    C->PrintJob(llvm::errs(), C->getJobs(), "\n", true);
    llvm::errs() << "\n";
  }

  // FIXME: This is copied from cc1_main.cpp; simplify and eliminate.

  // Create a compiler instance to handle the actual work.
  CompilerInstance LFort;
  LFort.setInvocation(CI.take());

  // Create the compilers actual diagnostics engine.
  LFort.createDiagnostics(int(CCArgs.size()),const_cast<char**>(CCArgs.data()));
  if (!LFort.hasDiagnostics())
    return 1;

  // Infer the builtin include path if unspecified.
  if (LFort.getHeaderSearchOpts().UseBuiltinIncludes &&
      LFort.getHeaderSearchOpts().ResourceDir.empty())
    LFort.getHeaderSearchOpts().ResourceDir =
      CompilerInvocation::GetResourcesPath(argv[0], MainAddr);

  // Create and execute the frontend to generate an LLVM bitcode module.
  OwningPtr<CodeGenAction> Act(new EmitLLVMOnlyAction());
  if (!LFort.ExecuteAction(*Act))
    return 1;

  int Res = 255;
  if (llvm::Module *PCModule = Act->takePCModule())
    Res = Execute(PCModule, envp);

  // Shutdown.

  llvm::llvm_shutdown();

  return Res;
}
