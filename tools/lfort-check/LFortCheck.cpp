//===--- tools/lfort-check/LFortCheck.cpp - LFort check tool --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file implements a lfort-check tool that runs lfort based on the info
//  stored in a compilation database.
//
//  This tool uses the LFort Tooling infrastructure, see
//    http://lfort.llvm.org/docs/HowToSetupToolingForLLVM.html
//  for details on setting it up with LLVM source tree.
//
//===----------------------------------------------------------------------===//

#include "lfort/AST/ASTConsumer.h"
#include "lfort/Driver/OptTable.h"
#include "lfort/Driver/Options.h"
#include "lfort/Frontend/ASTConsumers.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Rewrite/Frontend/FixItRewriter.h"
#include "lfort/Rewrite/Frontend/FrontendActions.h"
#include "lfort/Tooling/CommonOptionsParser.h"
#include "lfort/Tooling/Tooling.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Path.h"

using namespace lfort::driver;
using namespace lfort::tooling;
using namespace llvm;

static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);
static cl::extrahelp MoreHelp(
    "\tFor example, to run lfort-check on all files in a subtree of the\n"
    "\tsource tree, use:\n"
    "\n"
    "\t  find path/in/subtree -name '*.cpp'|xargs lfort-check\n"
    "\n"
    "\tor using a specific build path:\n"
    "\n"
    "\t  find path/in/subtree -name '*.cpp'|xargs lfort-check -p build/path\n"
    "\n"
    "\tNote, that path/in/subtree and current directory should follow the\n"
    "\trules described above.\n"
    "\n"
);

static OwningPtr<OptTable> Options(createDriverOptTable());
static cl::opt<bool> ASTDump(
    "ast-dump",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_dump)));
static cl::opt<bool> ASTList(
    "ast-list",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_list)));
static cl::opt<bool> ASTPrint(
    "ast-print",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_print)));
static cl::opt<std::string> ASTDumpFilter(
    "ast-dump-filter",
    cl::desc(Options->getOptionHelpText(options::OPT_ast_dump_filter)));

static cl::opt<bool> Fixit(
    "fixit",
    cl::desc(Options->getOptionHelpText(options::OPT_fixit)));
static cl::opt<bool> FixWhatYouCan(
    "fix-what-you-can",
    cl::desc(Options->getOptionHelpText(options::OPT_fix_what_you_can)));

namespace {

// FIXME: Move FixItRewriteInPlace from lib/Rewrite/Frontend/FrontendActions.cpp
// into a header file and reuse that.
class FixItOptions : public lfort::FixItOptions {
public:
  FixItOptions() {
    FixWhatYouCan = ::FixWhatYouCan;
  }

  std::string RewriteFilename(const std::string& filename, int &fd) {
    assert(llvm::sys::path::is_absolute(filename) &&
           "lfort-fixit expects absolute paths only.");

    // We don't need to do permission checking here since lfort will diagnose
    // any I/O errors itself.

    fd = -1;  // No file descriptor for file.

    return filename;
  }
};

/// \brief Subclasses \c lfort::FixItRewriter to not count fixed errors/warnings
/// in the final error counts.
///
/// This has the side-effect that lfort-check -fixit exits with code 0 on
/// successfully fixing all errors.
class FixItRewriter : public lfort::FixItRewriter {
public:
  FixItRewriter(lfort::DiagnosticsEngine& Diags,
                lfort::SourceManager& SourceMgr,
                const lfort::LangOptions& LangOpts,
                lfort::FixItOptions* FixItOpts)
      : lfort::FixItRewriter(Diags, SourceMgr, LangOpts, FixItOpts) {
  }

  virtual bool IncludeInDiagnosticCounts() const { return false; }
};

/// \brief Subclasses \c lfort::FixItAction so that we can install the custom
/// \c FixItRewriter.
class FixItAction : public lfort::FixItAction {
public:
  virtual bool BeginSourceFileAction(lfort::CompilerInstance& CI,
                                     StringRef Filename) {
    FixItOpts.reset(new FixItOptions);
    Rewriter.reset(new FixItRewriter(CI.getDiagnostics(), CI.getSourceManager(),
                                     CI.getLangOpts(), FixItOpts.get()));
    return true;
  }
};

} // namespace

// Anonymous namespace here causes problems with gcc <= 4.4 on MacOS 10.6.
// "Non-global symbol: ... can't be a weak_definition"
namespace lfort_check {
class LFortCheckActionFactory {
public:
  lfort::ASTConsumer *newASTConsumer() {
    if (ASTList)
      return lfort::CreateASTDeclNodeLister();
    if (ASTDump)
      return lfort::CreateASTDumper(ASTDumpFilter);
    if (ASTPrint)
      return lfort::CreateASTPrinter(&llvm::outs(), ASTDumpFilter);
    return new lfort::ASTConsumer();
  }
};
}

int main(int argc, const char **argv) {
  CommonOptionsParser OptionsParser(argc, argv);
  LFortTool Tool(OptionsParser.getCompilations(),
                 OptionsParser.getSourcePathList());
  if (Fixit)
    return Tool.run(newFrontendActionFactory<FixItAction>());
  lfort_check::LFortCheckActionFactory Factory;
  return Tool.run(newFrontendActionFactory(&Factory));
}
