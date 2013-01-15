//===- unittests/Frontend/FrontendActionTest.cpp - FrontendAction tests ---===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Frontend/FrontendAction.h"
#include "lfort/AST/ASTConsumer.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/RecursiveASTVisitor.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/CompilerInvocation.h"
#include "llvm/ADT/Triple.h"
#include "llvm/Support/MemoryBuffer.h"
#include "gtest/gtest.h"

using namespace llvm;
using namespace lfort;

namespace {

class TestASTFrontendAction : public ASTFrontendAction {
public:
  std::vector<std::string> decl_names;

  virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI,
                                         StringRef InFile) {
    return new Visitor(decl_names);
  }

private:
  class Visitor : public ASTConsumer, public RecursiveASTVisitor<Visitor> {
  public:
    Visitor(std::vector<std::string> &decl_names) : decl_names_(decl_names) {}

    virtual void HandleProgram(ASTContext &context) {
      TraverseDecl(context.getProgramDecl());
    }

    virtual bool VisitNamedDecl(NamedDecl *Decl) {
      decl_names_.push_back(Decl->getQualifiedNameAsString());
      return true;
    }

  private:
    std::vector<std::string> &decl_names_;
  };
};

TEST(ASTFrontendAction, Sanity) {
  CompilerInvocation *invocation = new CompilerInvocation;
  invocation->getPreprocessorOpts().addRemappedFile(
    "test.f90", MemoryBuffer::getMemBuffer("program test\nend program"));
  invocation->getFrontendOpts().Inputs.push_back(FrontendInputFile("test.f90",
                                                                   IK_Fortran90));
  invocation->getFrontendOpts().ProgramAction = frontend::ParseSyntaxOnly;
  invocation->getTargetOpts().Triple = "i386-unknown-linux-gnu";
  CompilerInstance compiler;
  compiler.setInvocation(invocation);
  compiler.createDiagnostics(0, NULL);

  TestASTFrontendAction test_action;
  ASSERT_TRUE(compiler.ExecuteAction(test_action));
  // FIXME: TODO
#if 0
  ASSERT_EQ(3U, test_action.decl_names.size());
  EXPECT_EQ("__builtin_va_list", test_action.decl_names[0]);
  EXPECT_EQ("main", test_action.decl_names[1]);
  EXPECT_EQ("x", test_action.decl_names[2]);
#endif
}

} // anonymous namespace
