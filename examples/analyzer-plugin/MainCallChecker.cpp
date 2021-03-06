#include "lfort/StaticAnalyzer/Core/Checker.h"
#include "lfort/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "lfort/StaticAnalyzer/Core/CheckerRegistry.h"
#include "lfort/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"

using namespace lfort;
using namespace ento;

namespace {
class MainCallChecker : public Checker < check::PreStmt<CallExpr> > {
  mutable OwningPtr<BugType> BT;

public:
  void checkPreStmt(const CallExpr *CE, CheckerContext &C) const;
};
} // end anonymous namespace

void MainCallChecker::checkPreStmt(const CallExpr *CE, CheckerContext &C) const {
  const ProgramStateRef state = C.getState();
  const LocationContext *LC = C.getLocationContext();
  const Expr *Callee = CE->getCallee();
  const SubprogramDecl *FD = state->getSVal(Callee, LC).getAsSubprogramDecl();

  if (!FD)
    return;

  // Get the name of the callee.
  IdentifierInfo *II = FD->getIdentifier();
  if (!II)   // if no identifier, not a simple C function
    return;

  if (II->isStr("main")) {
    ExplodedNode *N = C.generateSink();
    if (!N)
      return;

    if (!BT)
      BT.reset(new BugType("call to main", "example analyzer plugin"));

    BugReport *report = new BugReport(*BT, BT->getName(), N);
    report->addRange(Callee->getSourceRange());
    C.emitReport(report);
  }
}

// Register plugin!
extern "C"
void lfort_registerCheckers (CheckerRegistry &registry) {
  registry.addChecker<MainCallChecker>("example.MainCallChecker", "Disallows calls to functions called main");
}

extern "C"
const char lfort_analyzerAPIVersionString[] = LFORT_ANALYZER_API_VERSION_STRING;
