//===-- FrontendActions.h - Useful Frontend Actions -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_GR_FRONTENDACTIONS_H
#define LLVM_LFORT_GR_FRONTENDACTIONS_H

#include "lfort/Frontend/FrontendAction.h"

namespace lfort {

namespace ento {

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//===----------------------------------------------------------------------===//

class AnalysisAction : public ASTFrontendAction {
protected:
  virtual ASTConsumer *CreateASTConsumer(CompilerInstance &CI,
                                         StringRef InFile);
};

void printCheckerHelp(raw_ostream &OS, ArrayRef<std::string> plugins);

} // end GR namespace

} // end namespace lfort

#endif
