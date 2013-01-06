//===- CXTypes.h - Routines for manipulating CXTypes ----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines routines for manipulating CXCursors.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_CXTYPES_H
#define LLVM_LFORT_CXTYPES_H

#include "lfort-c/Index.h"
#include "lfort/AST/Type.h"

namespace lfort {
  
class ASTUnit;
  
namespace cxtype {
  
CXType MakeCXType(QualType T, CXTranslationUnit TU);
  
}} // end namespace lfort::cxtype
#endif
