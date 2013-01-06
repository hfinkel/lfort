//===--- DeclVisitor.h - Visitor for Decl subclasses ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the DeclVisitor interface.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LFORT_AST_DECLVISITOR_H
#define LLVM_LFORT_AST_DECLVISITOR_H

#include "lfort/AST/Decl.h"
#include "lfort/AST/DeclCXX.h"
#include "lfort/AST/DeclFriend.h"
#include "lfort/AST/DeclObjC.h"
#include "lfort/AST/DeclTemplate.h"

namespace lfort {

#define DISPATCH(NAME, CLASS) \
  return static_cast<ImplClass*>(this)-> Visit##NAME(static_cast<CLASS*>(D))

/// \brief A simple visitor class that helps create declaration visitors.
template<typename ImplClass, typename RetTy=void>
class DeclVisitor {
public:
  RetTy Visit(Decl *D) {
    switch (D->getKind()) {
#define DECL(DERIVED, BASE) \
      case Decl::DERIVED: DISPATCH(DERIVED##Decl, DERIVED##Decl);
#define ABSTRACT_DECL(DECL)
#include "lfort/AST/DeclNodes.inc"
    }
    llvm_unreachable("Decl that isn't part of DeclNodes.inc!");
  }

  // If the implementation chooses not to implement a certain visit
  // method, fall back to the parent.
#define DECL(DERIVED, BASE) \
  RetTy Visit##DERIVED##Decl(DERIVED##Decl *D) { DISPATCH(BASE, BASE); }
#include "lfort/AST/DeclNodes.inc"

  RetTy VisitDecl(Decl *D) { return RetTy(); }
};

#undef DISPATCH

}  // end namespace lfort

#endif // LLVM_LFORT_AST_DECLVISITOR_H
