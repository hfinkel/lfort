//===- DiagnosticNames.cpp - Defines a table of all builtin diagnostics ----==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "DiagnosticNames.h"
#include "lfort/Basic/AllDiagnostics.h"
#include "llvm/ADT/STLExtras.h"

using namespace lfort;
using namespace diagtool;

static const DiagnosticRecord BuiltinDiagnosticsByName[] = {
#define DIAG_NAME_INDEX(ENUM) { #ENUM, diag::ENUM, STR_SIZE(#ENUM, uint8_t) },
#include "lfort/Basic/DiagnosticIndexName.inc"
#undef DIAG_NAME_INDEX
};

llvm::ArrayRef<DiagnosticRecord> diagtool::getBuiltinDiagnosticsByName() {
  return llvm::makeArrayRef(BuiltinDiagnosticsByName);
}


// FIXME: Is it worth having two tables, especially when this one can get
// out of sync easily?
static const DiagnosticRecord BuiltinDiagnosticsByID[] = {
#define DIAG(ENUM,CLASS,DEFAULT_MAPPING,DESC,GROUP,               \
             SFINAE,ACCESS,NOWERROR,SHOWINSYSHEADER,              \
             CATEGORY)                                            \
  { #ENUM, diag::ENUM, STR_SIZE(#ENUM, uint8_t) },
#include "lfort/Basic/DiagnosticCommonKinds.inc"
#include "lfort/Basic/DiagnosticDriverKinds.inc"
#include "lfort/Basic/DiagnosticFrontendKinds.inc"
#include "lfort/Basic/DiagnosticSerializationKinds.inc"
#include "lfort/Basic/DiagnosticLexKinds.inc"
#include "lfort/Basic/DiagnosticParseKinds.inc"
#include "lfort/Basic/DiagnosticASTKinds.inc"
#include "lfort/Basic/DiagnosticCommentKinds.inc"
#include "lfort/Basic/DiagnosticSemaKinds.inc"
#include "lfort/Basic/DiagnosticAnalysisKinds.inc"
#undef DIAG
};

static bool orderByID(const DiagnosticRecord &Left,
                      const DiagnosticRecord &Right) {
  return Left.DiagID < Right.DiagID;
}

const DiagnosticRecord &diagtool::getDiagnosticForID(short DiagID) {
  DiagnosticRecord Key = {0, DiagID, 0};

  const DiagnosticRecord *Result =
    std::lower_bound(BuiltinDiagnosticsByID,
                     llvm::array_endof(BuiltinDiagnosticsByID),
                     Key, orderByID);
  assert(Result && "diagnostic not found; table may be out of date");
  return *Result;
}


#define GET_DIAG_ARRAYS
#include "lfort/Basic/DiagnosticGroups.inc"
#undef GET_DIAG_ARRAYS

// Second the table of options, sorted by name for fast binary lookup.
static const GroupRecord OptionTable[] = {
#define GET_DIAG_TABLE
#include "lfort/Basic/DiagnosticGroups.inc"
#undef GET_DIAG_TABLE
};

llvm::ArrayRef<GroupRecord> diagtool::getDiagnosticGroups() {
  return llvm::makeArrayRef(OptionTable);
}
