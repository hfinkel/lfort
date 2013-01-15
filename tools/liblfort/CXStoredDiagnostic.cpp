/*===-- CXStoreDiagnostic.cpp - Diagnostics C Interface ----------*- C++ -*-===*\
|*                                                                            *|
|*                     The LLVM Compiler Infrastructure                       *|
|*                                                                            *|
|* This file is distributed under the University of Illinois Open Source      *|
|* License. See LICENSE.TXT for details.                                      *|
|*                                                                            *|
|*===----------------------------------------------------------------------===*|
|*                                                                            *|
|* Implements part of the diagnostic functions of the LFort C interface.      *|
|*                                                                            *|
\*===----------------------------------------------------------------------===*/

#include "CIndexDiagnostic.h"
#include "CIndexer.h"
#include "CXProgram.h"
#include "CXSourceLocation.h"
#include "CXString.h"

#include "lfort/Frontend/ASTUnit.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "llvm/ADT/SmallString.h"
#include "llvm/ADT/Twine.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"

using namespace lfort;
using namespace lfort::cxloc;
using namespace lfort::cxstring;

CXDiagnosticSeverity CXStoredDiagnostic::getSeverity() const {
  switch (Diag.getLevel()) {
    case DiagnosticsEngine::Ignored: return CXDiagnostic_Ignored;
    case DiagnosticsEngine::Note:    return CXDiagnostic_Note;
    case DiagnosticsEngine::Warning: return CXDiagnostic_Warning;
    case DiagnosticsEngine::Error:   return CXDiagnostic_Error;
    case DiagnosticsEngine::Fatal:   return CXDiagnostic_Fatal;
  }
  
  llvm_unreachable("Invalid diagnostic level");
}

CXSourceLocation CXStoredDiagnostic::getLocation() const {
  if (Diag.getLocation().isInvalid())
    return lfort_getNullLocation();
  
  return translateSourceLocation(Diag.getLocation().getManager(),
                                 LangOpts, Diag.getLocation());
}

CXString CXStoredDiagnostic::getSpelling() const {
  return createCXString(Diag.getMessage(), false);
}

CXString CXStoredDiagnostic::getDiagnosticOption(CXString *Disable) const {
  unsigned ID = Diag.getID();
  StringRef Option = DiagnosticIDs::getWarningOptionForDiag(ID);
  if (!Option.empty()) {
    if (Disable)
      *Disable = createCXString((Twine("-Wno-") + Option).str());
    return createCXString((Twine("-W") + Option).str());
  }
  
  if (ID == diag::fatal_too_many_errors) {
    if (Disable)
      *Disable = createCXString("-ferror-limit=0");
    return createCXString("-ferror-limit=");
  }

  return createCXString("");
}

unsigned CXStoredDiagnostic::getCategory() const {
  return DiagnosticIDs::getCategoryNumberForDiag(Diag.getID());
}

CXString CXStoredDiagnostic::getCategoryText() const {
  unsigned catID = DiagnosticIDs::getCategoryNumberForDiag(Diag.getID());
  return createCXString(DiagnosticIDs::getCategoryNameFromID(catID));
}

unsigned CXStoredDiagnostic::getNumRanges() const {
  if (Diag.getLocation().isInvalid())
    return 0;
  
  return Diag.range_size();
}

CXSourceRange CXStoredDiagnostic::getRange(unsigned int Range) const {
  assert(Diag.getLocation().isValid());
  return translateSourceRange(Diag.getLocation().getManager(),
                              LangOpts,
                              Diag.range_begin()[Range]);
}

unsigned CXStoredDiagnostic::getNumFixIts() const {
  if (Diag.getLocation().isInvalid())
    return 0;    
  return Diag.fixit_size();
}

CXString CXStoredDiagnostic::getFixIt(unsigned FixIt,
                                      CXSourceRange *ReplacementRange) const {  
  const FixItHint &Hint = Diag.fixit_begin()[FixIt];
  if (ReplacementRange) {
    // Create a range that covers the entire replacement (or
    // removal) range, adjusting the end of the range to point to
    // the end of the token.
    *ReplacementRange = translateSourceRange(Diag.getLocation().getManager(),
                                             LangOpts, Hint.RemoveRange);
  }
  return createCXString(Hint.CodeToInsert);
}

