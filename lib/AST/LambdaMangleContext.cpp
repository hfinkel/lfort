//===--- LambdaMangleContext.cpp - Context for mangling lambdas -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the LambdaMangleContext class, which keeps track of
//  the Itanium C++ ABI mangling numbers for lambda expressions.
//
//===----------------------------------------------------------------------===//

#include "lfort/AST/LambdaMangleContext.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/DeclCXX.h"

using namespace lfort;

unsigned LambdaMangleContext::getManglingNumber(CXXMethodDecl *CallOperator) {
  const SubprogramProtoType *Proto
    = CallOperator->getType()->getAs<SubprogramProtoType>();
  ASTContext &Context = CallOperator->getASTContext();
  
  QualType Key = Context.getSubprogramType(Context.VoidTy, 
                                         Proto->arg_type_begin(),
                                         Proto->getNumArgs(),
                                         SubprogramProtoType::ExtProtoInfo());
  Key = Context.getCanonicalType(Key);
  return ++ManglingNumbers[Key->castAs<SubprogramProtoType>()];
}
