//===--- AllDiagnostics.h - Aggregate Diagnostic headers --------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Includes all the separate Diagnostic headers & some related helpers.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_ALL_DIAGNOSTICS_H
#define LLVM_LFORT_ALL_DIAGNOSTICS_H

#include "lfort/AST/ASTDiagnostic.h"
#include "lfort/AST/CommentDiagnostic.h"
#include "lfort/Analysis/AnalysisDiagnostic.h"
#include "lfort/Driver/DriverDiagnostic.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/Lex/LexDiagnostic.h"
#include "lfort/Parse/ParseDiagnostic.h"
#include "lfort/Sema/SemaDiagnostic.h"
#include "lfort/Serialization/SerializationDiagnostic.h"

namespace lfort {
template <size_t SizeOfStr, typename FieldType>
class StringSizerHelper {
  char FIELD_TOO_SMALL[SizeOfStr <= FieldType(~0U) ? 1 : -1];
public:
  enum { Size = SizeOfStr };
};
} // end namespace lfort 

#define STR_SIZE(str, fieldTy) lfort::StringSizerHelper<sizeof(str)-1, \
                                                        fieldTy>::Size 

#endif
