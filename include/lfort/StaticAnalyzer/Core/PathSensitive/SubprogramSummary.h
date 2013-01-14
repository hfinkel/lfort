//== SubprogramSummary.h - Stores summaries of functions. ------------*- C++ -*-//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines a summary of a function gathered/used by static analyzes.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_GR_FUNCTIONSUMMARY_H
#define LLVM_LFORT_GR_FUNCTIONSUMMARY_H

#include "lfort/AST/Decl.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/DenseSet.h"
#include <deque>

namespace lfort {
namespace ento {
typedef std::deque<Decl*> SetOfDecls;
typedef llvm::DenseSet<const Decl*> SetOfConstDecls;

class SubprogramSummariesTy {
  struct SubprogramSummary {
    /// True if this function has reached a max block count while inlined from
    /// at least one call site.
    bool MayReachMaxBlockCount;

    /// Total number of blocks in the function.
    unsigned TotalBasicBlocks;

    /// Marks the IDs of the basic blocks visited during the analyzes.
    llvm::BitVector VisitedBasicBlocks;

    /// The number of times the function has been inlined.
    unsigned TimesInlined;

    SubprogramSummary() :
      MayReachMaxBlockCount(false),
      TotalBasicBlocks(0),
      VisitedBasicBlocks(0),
      TimesInlined(0) {}
  };

  typedef llvm::DenseMap<const Decl*, SubprogramSummary*> MapTy;
  MapTy Map;

public:
  ~SubprogramSummariesTy();

  MapTy::iterator findOrInsertSummary(const Decl *D) {
    MapTy::iterator I = Map.find(D);
    if (I != Map.end())
      return I;
    SubprogramSummary *DS = new SubprogramSummary();
    I = Map.insert(std::pair<const Decl*, SubprogramSummary*>(D, DS)).first;
    assert(I != Map.end());
    return I;
  }

  void markReachedMaxBlockCount(const Decl* D) {
    MapTy::iterator I = findOrInsertSummary(D);
    I->second->MayReachMaxBlockCount = true;
  }

  bool hasReachedMaxBlockCount(const Decl* D) {
  MapTy::const_iterator I = Map.find(D);
    if (I != Map.end())
      return I->second->MayReachMaxBlockCount;
    return false;
  }

  void markVisitedBasicBlock(unsigned ID, const Decl* D, unsigned TotalIDs) {
    MapTy::iterator I = findOrInsertSummary(D);
    llvm::BitVector &Blocks = I->second->VisitedBasicBlocks;
    assert(ID < TotalIDs);
    if (TotalIDs > Blocks.size()) {
      Blocks.resize(TotalIDs);
      I->second->TotalBasicBlocks = TotalIDs;
    }
    Blocks[ID] = true;
  }

  unsigned getNumVisitedBasicBlocks(const Decl* D) {
    MapTy::const_iterator I = Map.find(D);
    if (I != Map.end())
      return I->second->VisitedBasicBlocks.count();
    return 0;
  }

  unsigned getNumTimesInlined(const Decl* D) {
    MapTy::const_iterator I = Map.find(D);
    if (I != Map.end())
      return I->second->TimesInlined;
    return 0;
  }

  void bumpNumTimesInlined(const Decl* D) {
    MapTy::iterator I = findOrInsertSummary(D);
    I->second->TimesInlined++;
  }

  /// Get the percentage of the reachable blocks.
  unsigned getPercentBlocksReachable(const Decl *D) {
    MapTy::const_iterator I = Map.find(D);
      if (I != Map.end())
        return ((I->second->VisitedBasicBlocks.count() * 100) /
                 I->second->TotalBasicBlocks);
    return 0;
  }

  unsigned getTotalNumBasicBlocks();
  unsigned getTotalNumVisitedBasicBlocks();

};

}} // end lfort ento namespaces

#endif
