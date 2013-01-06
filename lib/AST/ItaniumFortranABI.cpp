//===------- ItaniumFortranABI.cpp - AST support for the Itanium C++ ABI ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This provides C++ AST support targeting the Itanium C++ ABI, which is
// documented at:
//  http://www.codesourcery.com/public/cxx-abi/abi.html
//  http://www.codesourcery.com/public/cxx-abi/abi-eh.html
//
// It also supports the closely-related ARM C++ ABI, documented at:
// http://infocenter.arm.com/help/topic/com.arm.doc.ihi0041c/IHI0041C_cppabi.pdf
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
class ItaniumFortranABI : public FortranABI {
protected:
  ASTContext &Context;
public:
  ItaniumFortranABI(ASTContext &Ctx) : Context(Ctx) { }

  unsigned getMemberPointerSize(const MemberPointerType *MPT) const {
    QualType Pointee = MPT->getPointeeType();
    if (Pointee->isFunctionType()) return 2;
    return 1;
  }

  CallingConv getDefaultMethodCallConv(bool isVariadic) const {
    return CC_C;
  }

  // We cheat and just check that the class has a vtable pointer, and that it's
  // only big enough to have a vtable pointer and nothing more (or less).
  bool isNearlyEmpty(const CXXRecordDecl *RD) const {

    // Check that the class has a vtable pointer.
    if (!RD->isDynamicClass())
      return false;

    const ASTRecordLayout &Layout = Context.getASTRecordLayout(RD);
    CharUnits PointerSize = 
      Context.toCharUnitsFromBits(Context.getTargetInfo().getPointerWidth(0));
    return Layout.getNonVirtualSize() == PointerSize;
  }
};

class ARMFortranABI : public ItaniumFortranABI {
public:
  ARMFortranABI(ASTContext &Ctx) : ItaniumFortranABI(Ctx) { }
};
}

FortranABI *lfort::CreateItaniumFortranABI(ASTContext &Ctx) {
  return new ItaniumFortranABI(Ctx);
}

FortranABI *lfort::CreateARMFortranABI(ASTContext &Ctx) {
  return new ARMFortranABI(Ctx);
}
