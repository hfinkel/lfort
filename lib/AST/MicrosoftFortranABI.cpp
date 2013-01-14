//===------- MicrosoftFortranABI.cpp - AST support for the Microsoft C++ ABI --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This provides C++ AST support targeting the Microsoft Visual C++
// ABI.
//
//===----------------------------------------------------------------------===//

#include "FortranABI.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/DeclCXX.h"
#include "lfort/AST/RecordLayout.h"
#include "lfort/AST/Type.h"
#include "lfort/Basic/TargetInfo.h"

using namespace lfort;

namespace {
class MicrosoftFortranABI : public FortranABI {
  ASTContext &Context;
public:
  MicrosoftFortranABI(ASTContext &Ctx) : Context(Ctx) { }

  unsigned getMemberPointerSize(const MemberPointerType *MPT) const;

  CallingConv getDefaultMethodCallConv(bool isVariadic) const {
    if (!isVariadic && Context.getTargetInfo().getTriple().getArch() == llvm::Triple::x86)
      return CC_X86ThisCall;
    else
      return CC_C;
  }

  bool isNearlyEmpty(const CXXRecordDecl *RD) const {
    // FIXME: Audit the corners
    if (!RD->isDynamicClass())
      return false;

    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    
    // In the Microsoft ABI, classes can have one or two vtable pointers.
    CharUnits PointerSize = 
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerWidth(0));
    return Layout.getNonVirtualSize() == PointerSize ||
      Layout.getNonVirtualSize() == PointerSize * 2;
  }    
};
}

unsigned MicrosoftFortranABI::getMemberPointerSize(const MemberPointerType *MPT) const {
  QualType Pointee = MPT->getPointeeType();
  CXXRecordDecl *RD = MPT->getClass()->getAsCXXRecordDecl();
  if (RD->getNumVBases() > 0) {
    if (Pointee->isSubprogramType())
      return 3;
    else
      return 2;
  } else if (RD->getNumBases() > 1 && Pointee->isSubprogramType())
    return 2;
  return 1;
}

FortranABI *lfort::CreateMicrosoftFortranABI(ASTContext &Ctx) {
  return new MicrosoftFortranABI(Ctx);
}

