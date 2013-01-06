//= ObjCNoReturn.cpp - Handling of Cocoa APIs known not to return --*- C++ -*---
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements special handling of recognizing ObjC API hooks that
// do not return but aren't marked as such in API headers.
//
//===----------------------------------------------------------------------===//

#include "lfort/AST/ASTContext.h"
#include "lfort/AST/ExprObjC.h"
#include "lfort/Analysis/DomainSpecific/ObjCNoReturn.h"

using namespace lfort;

static bool isSubclass(const ObjCInterfaceDecl *Class, IdentifierInfo *II) {
  if (!Class)
    return false;
  if (Class->getIdentifier() == II)
    return true;
  return isSubclass(Class->getSuperClass(), II);
}

ObjCNoReturn::ObjCNoReturn(ASTContext &C)
  : RaiseSel(GetNullarySelector("raise", C)),
    NSExceptionII(&C.Idents.get("NSException"))
{
  // Generate selectors.
  SmallVector<IdentifierInfo*, 3> II;
  
  // raise:format:
  II.push_back(&C.Idents.get("raise"));
  II.push_back(&C.Idents.get("format"));
  NSExceptionInstanceRaiseSelectors[0] =
    C.Selectors.getSelector(II.size(), &II[0]);
    
  // raise:format:arguments:
  II.push_back(&C.Idents.get("arguments"));
  NSExceptionInstanceRaiseSelectors[1] =
    C.Selectors.getSelector(II.size(), &II[0]);
}


bool ObjCNoReturn::isImplicitNoReturn(const ObjCMessageExpr *ME) {
  Selector S = ME->getSelector();
  
  if (ME->isInstanceMessage()) {
    // Check for the "raise" message.
    return S == RaiseSel;
  }

  if (const ObjCInterfaceDecl *ID = ME->getReceiverInterface()) {
    if (isSubclass(ID, NSExceptionII)) {
      for (unsigned i = 0; i < NUM_RAISE_SELECTORS; ++i) {
        if (S == NSExceptionInstanceRaiseSelectors[i])
          return true;
      }
    }
  }
  
  return false;
}
