//===- unittests/Lex/PPConditionalDirectiveRecordTest.cpp-PP directive tests =//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Lex/PPConditionalDirectiveRecord.h"
#include "lfort/Basic/Diagnostic.h"
#include "lfort/Basic/DiagnosticOptions.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Basic/LangOptions.h"
#include "lfort/Basic/SourceManager.h"
#include "lfort/Basic/TargetInfo.h"
#include "lfort/Basic/TargetOptions.h"
#include "lfort/Lex/HeaderSearch.h"
#include "lfort/Lex/HeaderSearchOptions.h"
#include "lfort/Lex/ModuleLoader.h"
#include "lfort/Lex/Preprocessor.h"
#include "lfort/Lex/PreprocessorOptions.h"
#include "llvm/Config/config.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace lfort;

namespace {

// The test fixture.
class PPConditionalDirectiveRecordTest : public ::testing::Test {
protected:
  PPConditionalDirectiveRecordTest()
    : FileMgr(FileMgrOpts),
      DiagID(new DiagnosticIDs()),
      Diags(DiagID, new DiagnosticOptions, new IgnoringDiagConsumer()),
      SourceMgr(Diags, FileMgr),
      TargetOpts(new TargetOptions)
  {
    TargetOpts->Triple = "x86_64-apple-darwin11.1.0";
    Target = TargetInfo::CreateTargetInfo(Diags, &*TargetOpts);
  }

  FileSystemOptions FileMgrOpts;
  FileManager FileMgr;
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID;
  DiagnosticsEngine Diags;
  SourceManager SourceMgr;
  LangOptions LangOpts;
  IntrusiveRefCntPtr<TargetOptions> TargetOpts;
  IntrusiveRefCntPtr<TargetInfo> Target;
};

class VoidModuleLoader : public ModuleLoader {
  virtual ModuleLoadResult loadModule(SourceLocation ImportLoc, 
                                      ModuleIdPath Path,
                                      Module::NameVisibilityKind Visibility,
                                      bool IsInclusionDirective) {
    return ModuleLoadResult();
  }
};

// FIXME: This test currently does not work because the numbers look like
// statement labels. Disable for now; if the current mechanism remains, then
// make the numbers larger to avoid this problem.
#if 0
TEST_F(PPConditionalDirectiveRecordTest, PPRecAPI) {
  const char *source =
      "0 1\n"
      "#if 1\n"
      "2\n"
      "#ifndef BB\n"
      "3 4\n"
      "#else\n"
      "#endif\n"
      "5\n"
      "#endif\n"
      "6\n"
      "#if 1\n"
      "7\n"
      "#if 1\n"
      "#endif\n"
      "8\n"
      "#endif\n"
      "9\n";

  MemoryBuffer *buf = MemoryBuffer::getMemBuffer(source);
  SourceMgr.createMainFileIDForMemBuffer(buf);

  VoidModuleLoader ModLoader;
  HeaderSearch HeaderInfo(new HeaderSearchOptions, FileMgr, Diags, LangOpts, 
                          Target.getPtr());
  Preprocessor PP(new PreprocessorOptions(), Diags, LangOpts,Target.getPtr(),
                  SourceMgr, HeaderInfo, ModLoader,
                  /*IILookup =*/ 0,
                  /*OwnsHeaderSearch =*/false,
                  /*DelayInitialization =*/ false);
  PPConditionalDirectiveRecord *
    PPRec = new PPConditionalDirectiveRecord(SourceMgr);
  PP.addPPCallbacks(PPRec);
  PP.EnterMainSourceFile();

  std::vector<Token> toks;
  while (1) {
    Token tok;
    PP.Lex(tok);
    if (tok.is(tok::eof))
      break;
    toks.push_back(tok);
  }

  // Make sure we got the tokens that we expected.
  ASSERT_EQ(10U, toks.size());
  
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[1].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[2].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[3].getLocation(), toks[4].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[1].getLocation(), toks[5].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[2].getLocation(), toks[6].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[2].getLocation(), toks[5].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[6].getLocation())));
  EXPECT_TRUE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[2].getLocation(), toks[8].getLocation())));
  EXPECT_FALSE(PPRec->rangeIntersectsConditionalDirective(
                    SourceRange(toks[0].getLocation(), toks[9].getLocation())));

  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[0].getLocation(), toks[2].getLocation()));
  EXPECT_FALSE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[3].getLocation(), toks[4].getLocation()));
  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[1].getLocation(), toks[5].getLocation()));
  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[2].getLocation(), toks[0].getLocation()));
  EXPECT_FALSE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[4].getLocation(), toks[3].getLocation()));
  EXPECT_TRUE(PPRec->areInDifferentConditionalDirectiveRegion(
                    toks[5].getLocation(), toks[1].getLocation()));
}
#endif

} // anonymous namespace
