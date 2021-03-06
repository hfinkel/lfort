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

#include "lfort/StaticAnalyzer/Core/PathSensitive/SubprogramSummary.h"
using namespace lfort;
using namespace ento;

SubprogramSummariesTy::~SubprogramSummariesTy() {
  for (MapTy::iterator I = Map.begin(), E = Map.end(); I != E; ++I) {
    delete(I->second);
  }
}

unsigned SubprogramSummariesTy::getTotalNumBasicBlocks() {
  unsigned Total = 0;
  for (MapTy::iterator I = Map.begin(), E = Map.end(); I != E; ++I) {
    Total += I->second->TotalBasicBlocks;
  }
  return Total;
}

unsigned SubprogramSummariesTy::getTotalNumVisitedBasicBlocks() {
  unsigned Total = 0;
  for (MapTy::iterator I = Map.begin(), E = Map.end(); I != E; ++I) {
    Total += I->second->VisitedBasicBlocks.count();
  }
  return Total;
}
