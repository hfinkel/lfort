//== Checker.cpp - Registration mechanism for checkers -----------*- C++ -*--=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines Checker, used to create and register checkers.
//
//===----------------------------------------------------------------------===//

#include "lfort/StaticAnalyzer/Core/PathSensitive/ProgramState.h"
#include "lfort/StaticAnalyzer/Core/Checker.h"

using namespace lfort;
using namespace ento;

StringRef CheckerBase::getTagDescription() const {
  // FIXME: We want to return the package + name of the checker here.
  return "A Checker";  
}

void Checker<check::_VoidCheck, check::_VoidCheck, check::_VoidCheck,
             check::_VoidCheck, check::_VoidCheck, check::_VoidCheck,
             check::_VoidCheck, check::_VoidCheck, check::_VoidCheck,
             check::_VoidCheck, check::_VoidCheck, check::_VoidCheck,
             check::_VoidCheck, check::_VoidCheck, check::_VoidCheck,
             check::_VoidCheck, check::_VoidCheck, check::_VoidCheck
             >::anchor() { }
