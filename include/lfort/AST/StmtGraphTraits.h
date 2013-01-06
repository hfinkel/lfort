//===--- StmtGraphTraits.h - Graph Traits for the class Stmt ----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines a template specialization of llvm::GraphTraits to
//  treat ASTs (Stmt*) as graphs
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_AST_STMT_GRAPHTRAITS_H
#define LLVM_LFORT_AST_STMT_GRAPHTRAITS_H

#include "lfort/AST/Stmt.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/ADT/GraphTraits.h"

namespace llvm {

//template <typename T> struct GraphTraits;


template <> struct GraphTraits<lfort::Stmt*> {
  typedef lfort::Stmt                       NodeType;
  typedef lfort::Stmt::child_iterator       ChildIteratorType;
  typedef llvm::df_iterator<lfort::Stmt*>   nodes_iterator;

  static NodeType* getEntryNode(lfort::Stmt* S) { return S; }

  static inline ChildIteratorType child_begin(NodeType* N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static inline ChildIteratorType child_end(NodeType* N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(lfort::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(lfort::Stmt* S) {
    return df_end(S);
  }
};


template <> struct GraphTraits<const lfort::Stmt*> {
  typedef const lfort::Stmt                       NodeType;
  typedef lfort::Stmt::const_child_iterator       ChildIteratorType;
  typedef llvm::df_iterator<const lfort::Stmt*>   nodes_iterator;

  static NodeType* getEntryNode(const lfort::Stmt* S) { return S; }

  static inline ChildIteratorType child_begin(NodeType* N) {
    if (N) return N->child_begin();
    else return ChildIteratorType();
  }

  static inline ChildIteratorType child_end(NodeType* N) {
    if (N) return N->child_end();
    else return ChildIteratorType();
  }

  static nodes_iterator nodes_begin(const lfort::Stmt* S) {
    return df_begin(S);
  }

  static nodes_iterator nodes_end(const lfort::Stmt* S) {
    return df_end(S);
  }
};


} // end namespace llvm

#endif
