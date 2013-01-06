//===--- FrontendActions.cpp ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/StaticAnalyzer/Frontend/FrontendActions.h"
#include "AnalysisConsumer.h"
#include "lfort/Frontend/CompilerInstance.h"
using namespace lfort;
using namespace ento;

ASTConsumer *AnalysisAction::CreateASTConsumer(CompilerInstance &CI,
                                               StringRef InFile) {
  return CreateAnalysisConsumer(CI.getPreprocessor(),
                                CI.getFrontendOpts().OutputFile,
                                CI.getAnalyzerOpts(),
                                CI.getFrontendOpts().Plugins);
}

