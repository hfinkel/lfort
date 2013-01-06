//===--- SemaInternal.h - Internal Sema Interfaces --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides common API and #includes for the internal
// implementation of Sema.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_SEMA_SEMA_INTERNAL_H
#define LLVM_LFORT_SEMA_SEMA_INTERNAL_H

#include "lfort/AST/ASTContext.h"
#include "lfort/Sema/Sema.h"
#include "lfort/Sema/SemaDiagnostic.h"

namespace lfort {

inline PartialDiagnostic Sema::PDiag(unsigned DiagID) {
  return PartialDiagnostic(DiagID, Context.getDiagAllocator());
}

}

#endif
