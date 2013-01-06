//===--- LFortCheckers.h - Provides builtin checkers ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/StaticAnalyzer/Checkers/LFortCheckers.h"
#include "lfort/StaticAnalyzer/Core/CheckerRegistry.h"

// FIXME: This is only necessary as long as there are checker registration
// functions that do additional work besides mgr.registerChecker<CLASS>().
// The only checkers that currently do this are:
// - NSAutoreleasePoolChecker
// - NSErrorChecker
// - ObjCAtSyncChecker
// It's probably worth including this information in Checkers.td to minimize
// boilerplate code.
#include "LFortSACheckers.h"

using namespace lfort;
using namespace ento;

void ento::registerBuiltinCheckers(CheckerRegistry &registry) {
#define GET_CHECKERS
#define CHECKER(FULLNAME,CLASS,DESCFILE,HELPTEXT,GROUPINDEX,HIDDEN)    \
  registry.addChecker(register##CLASS, FULLNAME, HELPTEXT);
#include "Checkers.inc"
#undef GET_CHECKERS
}
