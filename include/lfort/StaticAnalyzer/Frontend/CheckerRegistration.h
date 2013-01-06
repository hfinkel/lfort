//===-- CheckerRegistration.h - Checker Registration Function ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_SA_FRONTEND_CHECKERREGISTRATION_H
#define LLVM_LFORT_SA_FRONTEND_CHECKERREGISTRATION_H

#include "lfort/Basic/LLVM.h"
#include <string>

namespace lfort {
  class AnalyzerOptions;
  class LangOptions;
  class DiagnosticsEngine;

namespace ento {
  class CheckerManager;

CheckerManager *createCheckerManager(const AnalyzerOptions &opts,
                                     const LangOptions &langOpts,
                                     ArrayRef<std::string> plugins,
                                     DiagnosticsEngine &diags);

} // end ento namespace

} // end namespace lfort

#endif
