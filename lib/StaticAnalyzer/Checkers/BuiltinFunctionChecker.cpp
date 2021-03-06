//=== BuiltinSubprogramChecker.cpp --------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This checker evaluates lfort builtin functions.
//
//===----------------------------------------------------------------------===//

#include "LFortSACheckers.h"
#include "lfort/Basic/Builtins.h"
#include "lfort/StaticAnalyzer/Core/Checker.h"
#include "lfort/StaticAnalyzer/Core/CheckerManager.h"
#include "lfort/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace lfort;
using namespace ento;

namespace {

class BuiltinSubprogramChecker : public Checker<eval::Call> {
public:
  bool evalCall(const CallExpr *CE, CheckerContext &C) const;
};

}

bool BuiltinSubprogramChecker::evalCall(const CallExpr *CE,
                                      CheckerContext &C) const {
  ProgramStateRef state = C.getState();
  const SubprogramDecl *FD = C.getCalleeDecl(CE);
  const LocationContext *LCtx = C.getLocationContext();
  if (!FD)
    return false;

  unsigned id = FD->getBuiltinID();

  if (!id)
    return false;

  switch (id) {
  case Builtin::BI__builtin_expect: {
    // For __builtin_expect, just return the value of the subexpression.
    assert (CE->arg_begin() != CE->arg_end());
    SVal X = state->getSVal(*(CE->arg_begin()), LCtx);
    C.addTransition(state->BindExpr(CE, LCtx, X));
    return true;
  }

  case Builtin::BI__builtin_alloca: {
    // FIXME: Refactor into StoreManager itself?
    MemRegionManager& RM = C.getStoreManager().getRegionManager();
    const AllocaRegion* R =
      RM.getAllocaRegion(CE, C.blockCount(), C.getLocationContext());

    // Set the extent of the region in bytes. This enables us to use the
    // SVal of the argument directly. If we save the extent in bits, we
    // cannot represent values like symbol*8.
    DefinedOrUnknownSVal Size =
      cast<DefinedOrUnknownSVal>(state->getSVal(*(CE->arg_begin()), LCtx));

    SValBuilder& svalBuilder = C.getSValBuilder();
    DefinedOrUnknownSVal Extent = R->getExtent(svalBuilder);
    DefinedOrUnknownSVal extentMatchesSizeArg =
      svalBuilder.evalEQ(state, Extent, Size);
    state = state->assume(extentMatchesSizeArg, true);
    assert(state && "The region should not have any previous constraints");

    C.addTransition(state->BindExpr(CE, LCtx, loc::MemRegionVal(R)));
    return true;
  }
  }

  return false;
}

void ento::registerBuiltinSubprogramChecker(CheckerManager &mgr) {
  mgr.registerChecker<BuiltinSubprogramChecker>();
}
