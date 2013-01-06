//===- ReachableCode.h -----------------------------------------*- C++ --*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// A flow-sensitive, path-insensitive analysis of unreachable code.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_REACHABLECODE_H
#define LLVM_LFORT_REACHABLECODE_H

#include "lfort/Basic/SourceLocation.h"

//===----------------------------------------------------------------------===//
// Forward declarations.
//===----------------------------------------------------------------------===//

namespace llvm {
  class BitVector;
}

namespace lfort {
  class AnalysisDeclContext;
  class CFGBlock;
}

//===----------------------------------------------------------------------===//
// API.
//===----------------------------------------------------------------------===//

namespace lfort {
namespace reachable_code {

class Callback {
  virtual void anchor();
public:
  virtual ~Callback() {}
  virtual void HandleUnreachable(SourceLocation L, SourceRange R1,
                                 SourceRange R2) = 0;
};

/// ScanReachableFromBlock - Mark all blocks reachable from Start.
/// Returns the total number of blocks that were marked reachable.  
unsigned ScanReachableFromBlock(const CFGBlock *Start,
                                llvm::BitVector &Reachable);

void FindUnreachableCode(AnalysisDeclContext &AC, Callback &CB);

}} // end namespace lfort::reachable_code

#endif
