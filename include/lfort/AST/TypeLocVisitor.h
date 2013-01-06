//===--- TypeLocVisitor.h - Visitor for TypeLoc subclasses ------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TypeLocVisitor interface.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LFORT_AST_TYPELOCVISITOR_H
#define LLVM_LFORT_AST_TYPELOCVISITOR_H

#include "lfort/AST/TypeLoc.h"
#include "lfort/AST/TypeVisitor.h"
#include "llvm/Support/ErrorHandling.h"

namespace lfort {

#define DISPATCH(CLASSNAME) \
  return static_cast<ImplClass*>(this)-> \
    Visit##CLASSNAME(cast<CLASSNAME>(TyLoc))

template<typename ImplClass, typename RetTy=void>
class TypeLocVisitor {
public:
  RetTy Visit(TypeLoc TyLoc) {
    switch (TyLoc.getTypeLocClass()) {
#define ABSTRACT_TYPELOC(CLASS, PARENT)
#define TYPELOC(CLASS, PARENT) \
    case TypeLoc::CLASS: DISPATCH(CLASS##TypeLoc);
#include "lfort/AST/TypeLocNodes.def"
    }
    llvm_unreachable("unexpected type loc class!");
  }

  RetTy Visit(UnqualTypeLoc TyLoc) {
    switch (TyLoc.getTypeLocClass()) {
#define ABSTRACT_TYPELOC(CLASS, PARENT)
#define TYPELOC(CLASS, PARENT) \
    case TypeLoc::CLASS: DISPATCH(CLASS##TypeLoc);
#include "lfort/AST/TypeLocNodes.def"
    }
    llvm_unreachable("unexpected type loc class!");
  }

#define TYPELOC(CLASS, PARENT)      \
  RetTy Visit##CLASS##TypeLoc(CLASS##TypeLoc TyLoc) { \
    DISPATCH(PARENT);               \
  }
#include "lfort/AST/TypeLocNodes.def"

  RetTy VisitTypeLoc(TypeLoc TyLoc) { return RetTy(); }
};

#undef DISPATCH

}  // end namespace lfort

#endif // LLVM_LFORT_AST_TYPELOCVISITOR_H
