//===- unittest/Tooling/ToolingTest.cpp - Tooling unit tests --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/AST/ASTConsumer.h"
#include "lfort/AST/DeclCXX.h"
#include "lfort/AST/DeclGroup.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/FrontendAction.h"
#include "lfort/Frontend/FrontendActions.h"
#include "lfort/Tooling/CompilationDatabase.h"
#include "lfort/Tooling/Tooling.h"
#include "gtest/gtest.h"
#include <string>

namespace lfort {
namespace tooling {

namespace {
/// Takes an ast consumer and returns it from CreateASTConsumer. This only
/// works with single translation unit compilations.
class TestAction : public lfort::ASTFrontendAction {
 public:
  /// Takes ownership of TestConsumer.
  explicit TestAction(lfort::ASTConsumer *TestConsumer)
      : TestConsumer(TestConsumer) {}

 protected:
  virtual lfort::ASTConsumer* CreateASTConsumer(
      lfort::CompilerInstance& compiler, StringRef dummy) {
    /// TestConsumer will be deleted by the framework calling us.
    return TestConsumer;
  }

 private:
  lfort::ASTConsumer * const TestConsumer;
};

class FindTopLevelDeclConsumer : public lfort::ASTConsumer {
 public:
  explicit FindTopLevelDeclConsumer(bool *FoundTopLevelDecl)
      : FoundTopLevelDecl(FoundTopLevelDecl) {}
  virtual bool HandleTopLevelDecl(lfort::DeclGroupRef DeclGroup) {
    *FoundTopLevelDecl = true;
    return true;
  }
 private:
  bool * const FoundTopLevelDecl;
};
} // end namespace

// FIXME: Convert to Fortran
#if 0
TEST(runToolOnCode, FindsNoTopLevelDeclOnEmptyCode) {
  bool FoundTopLevelDecl = false;
  EXPECT_TRUE(runToolOnCode(
      new TestAction(new FindTopLevelDeclConsumer(&FoundTopLevelDecl)), ""));
#if !defined(_MSC_VER)
  EXPECT_FALSE(FoundTopLevelDecl);
#else
  // FIXME: LangOpts.MicrosoftExt appends "class type_info;"
  EXPECT_TRUE(FoundTopLevelDecl);
#endif
}
#endif

namespace {
class FindClassDeclXConsumer : public lfort::ASTConsumer {
 public:
  FindClassDeclXConsumer(bool *FoundClassDeclX)
      : FoundClassDeclX(FoundClassDeclX) {}
  virtual bool HandleTopLevelDecl(lfort::DeclGroupRef GroupRef) {
    if (CXXRecordDecl* Record = dyn_cast<lfort::CXXRecordDecl>(
            *GroupRef.begin())) {
      if (Record->getName() == "X") {
        *FoundClassDeclX = true;
      }
    }
    return true;
  }
 private:
  bool *FoundClassDeclX;
};
} // end namespace

// FIXME: Convert to Fortran
#if 0
TEST(runToolOnCode, FindsClassDecl) {
  bool FoundClassDeclX = false;
  EXPECT_TRUE(runToolOnCode(new TestAction(
      new FindClassDeclXConsumer(&FoundClassDeclX)), "class X;"));
  EXPECT_TRUE(FoundClassDeclX);

  FoundClassDeclX = false;
  EXPECT_TRUE(runToolOnCode(new TestAction(
      new FindClassDeclXConsumer(&FoundClassDeclX)), "class Y;"));
  EXPECT_FALSE(FoundClassDeclX);
}
#endif

TEST(newFrontendActionFactory, CreatesFrontendActionFactoryFromType) {
  llvm::OwningPtr<FrontendActionFactory> Factory(
    newFrontendActionFactory<SyntaxOnlyAction>());
  llvm::OwningPtr<FrontendAction> Action(Factory->create());
  EXPECT_TRUE(Action.get() != NULL);
}

struct IndependentFrontendActionCreator {
  ASTConsumer *newASTConsumer() {
    return new FindTopLevelDeclConsumer(NULL);
  }
};

TEST(newFrontendActionFactory, CreatesFrontendActionFactoryFromFactoryType) {
  IndependentFrontendActionCreator Creator;
  llvm::OwningPtr<FrontendActionFactory> Factory(
    newFrontendActionFactory(&Creator));
  llvm::OwningPtr<FrontendAction> Action(Factory->create());
  EXPECT_TRUE(Action.get() != NULL);
}

TEST(ToolInvocation, TestMapVirtualFile) {
  lfort::FileManager Files((lfort::FileSystemOptions()));
  std::vector<std::string> Args;
  Args.push_back("tool-executable");
  Args.push_back("-Idef");
  Args.push_back("-fsyntax-only");
  Args.push_back("test.F90");
  lfort::tooling::ToolInvocation Invocation(Args, new SyntaxOnlyAction, &Files);
  Invocation.mapVirtualFile("test.F90", "#include <abc>\n");
  Invocation.mapVirtualFile("def/abc", "\n");
  EXPECT_TRUE(Invocation.run());
}

struct VerifyEndCallback : public EndOfSourceFileCallback {
  VerifyEndCallback() : Called(0), Matched(false) {}
  virtual void run() {
    ++Called;
  }
  ASTConsumer *newASTConsumer() {
    return new FindTopLevelDeclConsumer(&Matched);
  }
  unsigned Called;
  bool Matched;
};

#if !defined(_WIN32)
TEST(newFrontendActionFactory, InjectsEndOfSourceFileCallback) {
  VerifyEndCallback EndCallback;

  FixedCompilationDatabase Compilations("/", std::vector<std::string>());
  std::vector<std::string> Sources;
  Sources.push_back("/a.f90");
  Sources.push_back("/b.f90");
  LFortTool Tool(Compilations, Sources);

  Tool.mapVirtualFile("/a.f90", "end");
  Tool.mapVirtualFile("/b.f90", "end");

  Tool.run(newFrontendActionFactory(&EndCallback, &EndCallback));

  EXPECT_TRUE(EndCallback.Matched);
  EXPECT_EQ(2u, EndCallback.Called);
}
#endif

struct SkipBodyConsumer : public lfort::ASTConsumer {
  /// Skip the 'skipMe' function.
  virtual bool shouldSkipSubprogramBody(Decl *D) {
    SubprogramDecl *F = dyn_cast<SubprogramDecl>(D);
    return F && F->getNameAsString() == "skipMe";
  }
};

struct SkipBodyAction : public lfort::ASTFrontendAction {
  virtual ASTConsumer *CreateASTConsumer(CompilerInstance &Compiler,
                                         StringRef) {
    Compiler.getFrontendOpts().SkipSubprogramBodies = true;
    return new SkipBodyConsumer;
  }
};

// FIXME: Convert to Fortran
#if 0
TEST(runToolOnCode, TestSkipSubprogramBody) {
  EXPECT_TRUE(runToolOnCode(new SkipBodyAction,
                            "int skipMe() { an_error_here }"));
  EXPECT_FALSE(runToolOnCode(new SkipBodyAction,
                             "int skipMeNot() { an_error_here }"));
}
#endif

} // end namespace tooling
} // end namespace lfort
