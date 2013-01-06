//===--- DiagnosticFrontend.h - Diagnostics for frontend --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_FRONTENDDIAGNOSTIC_H
#define LLVM_LFORT_FRONTENDDIAGNOSTIC_H

#include "lfort/Basic/Diagnostic.h"

namespace lfort {
  namespace diag {
    enum {
#define DIAG(ENUM,FLAGS,DEFAULT_MAPPING,DESC,GROUP,\
             SFINAE,ACCESS,NOWERROR,SHOWINSYSHEADER,CATEGORY) ENUM,
#define FRONTENDSTART
#include "lfort/Basic/DiagnosticFrontendKinds.inc"
#undef DIAG
      NUM_BUILTIN_FRONTEND_DIAGNOSTICS
    };
  }  // end namespace diag
}  // end namespace lfort

#endif
