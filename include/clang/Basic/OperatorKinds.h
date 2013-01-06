//===--- OperatorKinds.h - C++ Overloaded Operators -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines an enumeration for C++ overloaded operators.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_BASIC_OPERATOR_KINDS_H
#define LLVM_LFORT_BASIC_OPERATOR_KINDS_H

namespace lfort {

/// \brief Enumeration specifying the different kinds of C++ overloaded
/// operators.
enum OverloadedOperatorKind {
  OO_None,                ///< Not an overloaded operator
#define OVERLOADED_OPERATOR(Name,Spelling,Token,Unary,Binary,MemberOnly) \
  OO_##Name,
#include "lfort/Basic/OperatorKinds.def"
  NUM_OVERLOADED_OPERATORS
};

/// \brief Retrieve the spelling of the given overloaded operator, without 
/// the preceding "operator" keyword.
const char *getOperatorSpelling(OverloadedOperatorKind Operator);
  
} // end namespace lfort

#endif
