//===--- FrontendOptions.cpp ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Frontend/FrontendOptions.h"
#include "llvm/ADT/StringSwitch.h"
using namespace lfort;

InputKind FrontendOptions::getInputKindForExtension(StringRef Extension) {
  return llvm::StringSwitch<InputKind>(Extension)
    .Case("ast", IK_AST)
    .Case("f90", IK_PreprocessedFortran90)
    .Case("f95", IK_PreprocessedFortran95)
    .Case("f03", IK_PreprocessedFortran03)
    .Case("f08", IK_PreprocessedFortran08)
    .Case("F90", IK_Fortran90)
    .Case("F95", IK_Fortran95)
    .Case("F03", IK_Fortran03)
    .Case("F08", IK_Fortran08)
    .Cases("f", "for", "fpp", IK_PreprocessedFortran77)
    .Cases("F", "FOR", "FPP", IK_Fortran77)
    .Cases("ll", "bc", IK_LLVM_IR)
    .Default(IK_Fortran08);
}
