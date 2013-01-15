//===- CXComment.h - Routines for manipulating CXComments -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines routines for manipulating CXComments.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_CXCOMMENT_H
#define LLVM_LFORT_CXCOMMENT_H

#include "CXProgram.h"
#include "lfort-c/Index.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/Comment.h"
#include "lfort/Frontend/ASTUnit.h"

namespace lfort {
namespace comments {
  class CommandTraits;
}

namespace cxcomment {

inline CXComment createCXComment(const comments::Comment *C,
                                 CXProgram Pgm) {
  CXComment Result;
  Result.ASTNode = C;
  Result.Program = Pgm;
  return Result;
}

inline const comments::Comment *getASTNode(CXComment CXC) {
  return static_cast<const comments::Comment *>(CXC.ASTNode);
}

template<typename T>
inline const T *getASTNodeAs(CXComment CXC) {
  const comments::Comment *C = getASTNode(CXC);
  if (!C)
    return NULL;

  return dyn_cast<T>(C);
}

inline ASTContext &getASTContext(CXComment CXC) {
  return static_cast<ASTUnit *>(CXC.Program->PgmData)->getASTContext();
}

inline comments::CommandTraits &getCommandTraits(CXComment CXC) {
  return getASTContext(CXC).getCommentCommandTraits();
}

} // end namespace cxcomment
} // end namespace lfort

#endif

