//=- AnalysisBasedWarnings.h - Sema warnings based on libAnalysis -*- C++ -*-=//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines AnalysisBasedWarnings, a worker object used by Sema
// that issues warnings based on dataflow-analysis.
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_SEMA_ANALYSIS_WARNINGS_H
#define LLVM_LFORT_SEMA_ANALYSIS_WARNINGS_H

#include "llvm/ADT/DenseMap.h"

namespace lfort {

class BlockExpr;
class Decl;
class SubprogramDecl;
class ObjCMethodDecl;
class QualType;
class Sema;
namespace sema {
  class SubprogramScopeInfo;
}

namespace sema {

class AnalysisBasedWarnings {
public:
  class Policy {
    friend class AnalysisBasedWarnings;
    // The warnings to run.
    unsigned enableCheckFallThrough : 1;
    unsigned enableCheckUnreachable : 1;
    unsigned enableThreadSafetyAnalysis : 1;
  public:
    Policy();
    void disableCheckFallThrough() { enableCheckFallThrough = 0; }
  };

private:
  Sema &S;
  Policy DefaultPolicy;

  enum VisitFlag { NotVisited = 0, Visited = 1, Pending = 2 };
  llvm::DenseMap<const SubprogramDecl*, VisitFlag> VisitedFD;

  /// \name Statistics
  /// @{

  /// \brief Number of function CFGs built and analyzed.
  unsigned NumSubprogramsAnalyzed;

  /// \brief Number of functions for which the CFG could not be successfully
  /// built.
  unsigned NumSubprogramsWithBadCFGs;

  /// \brief Total number of blocks across all CFGs.
  unsigned NumCFGBlocks;

  /// \brief Largest number of CFG blocks for a single function analyzed.
  unsigned MaxCFGBlocksPerSubprogram;

  /// \brief Total number of CFGs with variables analyzed for uninitialized
  /// uses.
  unsigned NumUninitAnalysisSubprograms;

  /// \brief Total number of variables analyzed for uninitialized uses.
  unsigned NumUninitAnalysisVariables;

  /// \brief Max number of variables analyzed for uninitialized uses in a single
  /// function.
  unsigned MaxUninitAnalysisVariablesPerSubprogram;

  /// \brief Total number of block visits during uninitialized use analysis.
  unsigned NumUninitAnalysisBlockVisits;

  /// \brief Max number of block visits during uninitialized use analysis of
  /// a single function.
  unsigned MaxUninitAnalysisBlockVisitsPerSubprogram;

  /// @}

public:
  AnalysisBasedWarnings(Sema &s);

  void IssueWarnings(Policy P, SubprogramScopeInfo *fscope,
                     const Decl *D, const BlockExpr *blkExpr);

  Policy getDefaultPolicy() { return DefaultPolicy; }

  void PrintStats() const;
};

}} // end namespace lfort::sema

#endif
