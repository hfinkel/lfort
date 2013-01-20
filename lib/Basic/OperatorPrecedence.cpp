//===--- OperatorPrecedence.cpp ---------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines and computes precedence levels for binary/ternary operators.
///
//===----------------------------------------------------------------------===//
#include "lfort/Basic/OperatorPrecedence.h"

namespace lfort {

prec::Level getBinOpPrecedence(tok::TokenKind Kind, bool GreaterThanIsOperator,
                               bool CPlusPlus11) {
  switch (Kind) {
  case tok::greater:
    // C++ [temp.names]p3:
    //   [...] When parsing a template-argument-list, the first
    //   non-nested > is taken as the ending delimiter rather than a
    //   greater-than operator. [...]
    if (GreaterThanIsOperator)
      return prec::Relational;
    return prec::Unknown;

  case tok::greatergreater:
    // C++0x [temp.names]p3:
    //
    //   [...] Similarly, the first non-nested >> is treated as two
    //   consecutive but distinct > tokens, the first of which is
    //   taken as the end of the template-argument-list and completes
    //   the template-id. [...]
    if (GreaterThanIsOperator || !CPlusPlus11)
      return prec::Shift;
    return prec::Unknown;

  default:                        return prec::Unknown;
  case tok::comma:                return prec::Comma;
  case tok::equalgreater:
  case tok::equal:                return prec::Assignment;
  case tok::kw_dotordot:
  case tok::pipepipe:             return prec::LogicalOr;
  case tok::kw_dotanddot:
  case tok::ampamp:               return prec::LogicalAnd;
  case tok::pipe:                 return prec::InclusiveOr;
  case tok::caret:                return prec::ExclusiveOr;
  case tok::amp:                  return prec::And;
  case tok::kw_doteqdot:
  case tok::kw_dotnedot:
  case tok::slashequal:
  case tok::equalequal:           return prec::Equality;
  case tok::kw_dotltdot:
  case tok::kw_dotledot:
  case tok::kw_dotgtdot:
  case tok::kw_dotgedot:
  case tok::lessequal:
  case tok::less:
  case tok::greaterequal:         return prec::Relational;
  case tok::lessless:             return prec::Shift;
  case tok::plus:
  case tok::minus:                return prec::Additive;
  case tok::slash:
  case tok::star:                 return prec::Multiplicative;
  }
}

}  // namespace lfort
