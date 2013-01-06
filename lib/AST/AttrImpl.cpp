//===--- AttrImpl.cpp - Classes for representing attributes -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file contains out-of-line virtual methods for Attr classes.
//
//===----------------------------------------------------------------------===//

#include "lfort/AST/Attr.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/Expr.h"
#include "lfort/AST/Type.h"
using namespace lfort;

Attr::~Attr() { }

void InheritableAttr::anchor() { }

void InheritableParamAttr::anchor() { }

#include "lfort/AST/AttrImpl.inc"
