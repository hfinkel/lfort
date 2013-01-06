//===--- MicrosoftFortranABI.cpp - Emit LLVM Code from ASTs for a Module ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This provides C++ code generation targeting the Microsoft Visual C++ ABI.
// The class in this file generates structures that follow the Microsoft
// Visual C++ ABI, which is actually not very well documented at all outside
// of Microsoft.
//
//===----------------------------------------------------------------------===//

#include "CGFortranABI.h"
#include "CodeGenModule.h"
#include "lfort/AST/Decl.h"
#include "lfort/AST/DeclCXX.h"

using namespace lfort;
using namespace CodeGen;

namespace {

class MicrosoftFortranABI : public CGFortranABI {
public:
  MicrosoftFortranABI(CodeGenModule &CGM) : CGFortranABI(CGM) {}

  StringRef GetPureVirtualCallName() { return "_purecall"; }
  // No known support for deleted functions in MSVC yet, so this choice is
  // arbitrary.
  StringRef GetDeletedVirtualCallName() { return "_purecall"; }

  llvm::Value *adjustToCompleteObject(CodeGenFunction &CGF,
                                      llvm::Value *ptr,
                                      QualType type);

  void BuildConstructorSignature(const CXXConstructorDecl *Ctor,
                                 CXXCtorType Type,
                                 CanQualType &ResTy,
                                 SmallVectorImpl<CanQualType> &ArgTys);

  void BuildDestructorSignature(const CXXDestructorDecl *Ctor,
                                CXXDtorType Type,
                                CanQualType &ResTy,
                                SmallVectorImpl<CanQualType> &ArgTys) {
    // 'this' is already in place
    // TODO: 'for base' flag
  }

  void BuildInstanceFunctionParams(CodeGenFunction &CGF,
                                   QualType &ResTy,
                                   FunctionArgList &Params);

  void EmitInstanceFunctionProlog(CodeGenFunction &CGF);

  void EmitGuardedInit(CodeGenFunction &CGF, const VarDecl &D,
                       llvm::GlobalVariable *DeclPtr,
                       bool PerformInit);

  void EmitVTables(const CXXRecordDecl *Class);


  // ==== Notes on array cookies =========
  //
  // MSVC seems to only use cookies when the class has a destructor; a
  // two-argument usual array deallocation function isn't sufficient.
  //
  // For example, this code prints "100" and "1":
  //   struct A {
  //     char x;
  //     void *operator new[](size_t sz) {
  //       printf("%u\n", sz);
  //       return malloc(sz);
  //     }
  //     void operator delete[](void *p, size_t sz) {
  //       printf("%u\n", sz);
  //       free(p);
  //     }
  //   };
  //   int main() {
  //     A *p = new A[100];
  //     delete[] p;
  //   }
  // Whereas it prints "104" and "104" if you give A a destructor.

  bool requiresArrayCookie(const CXXDeleteExpr *expr, QualType elementType);
  bool requiresArrayCookie(const CXXNewExpr *expr);
  CharUnits getArrayCookieSizeImpl(QualType type);
  llvm::Value *InitializeArrayCookie(CodeGenFunction &CGF,
                                     llvm::Value *NewPtr,
                                     llvm::Value *NumElements,
                                     const CXXNewExpr *expr,
                                     QualType ElementType);
  llvm::Value *readArrayCookieImpl(CodeGenFunction &CGF,
                                   llvm::Value *allocPtr,
                                   CharUnits cookieSize);
  static bool needThisReturn(GlobalDecl GD);
};

}

llvm::Value *MicrosoftFortranABI::adjustToCompleteObject(CodeGenFunction &CGF,
                                                     llvm::Value *ptr,
                                                     QualType type) {
  // FIXME: implement
  return ptr;
}

bool MicrosoftFortranABI::needThisReturn(GlobalDecl GD) {
  const CXXMethodDecl* MD = cast<CXXMethodDecl>(GD.getDecl());
  return isa<CXXConstructorDecl>(MD);
}

void MicrosoftFortranABI::BuildConstructorSignature(const CXXConstructorDecl *Ctor,
                                 CXXCtorType Type,
                                 CanQualType &ResTy,
                                 SmallVectorImpl<CanQualType> &ArgTys) {
  // 'this' is already in place
  // TODO: 'for base' flag
  // Ctor returns this ptr
  ResTy = ArgTys[0];
}

void MicrosoftFortranABI::BuildInstanceFunctionParams(CodeGenFunction &CGF,
                                                  QualType &ResTy,
                                                  FunctionArgList &Params) {
  BuildThisParam(CGF, Params);
  if (needThisReturn(CGF.CurGD)) {
    ResTy = Params[0]->getType();
  }
}

void MicrosoftFortranABI::EmitInstanceFunctionProlog(CodeGenFunction &CGF) {
  EmitThisParam(CGF);
  if (needThisReturn(CGF.CurGD)) {
    CGF.Builder.CreateStore(getThisValue(CGF), CGF.ReturnValue);
  }
}

bool MicrosoftFortranABI::requiresArrayCookie(const CXXDeleteExpr *expr,
                                   QualType elementType) {
  // Microsoft seems to completely ignore the possibility of a
  // two-argument usual deallocation function.
  return elementType.isDestructedType();
}

bool MicrosoftFortranABI::requiresArrayCookie(const CXXNewExpr *expr) {
  // Microsoft seems to completely ignore the possibility of a
  // two-argument usual deallocation function.
  return expr->getAllocatedType().isDestructedType();
}

CharUnits MicrosoftFortranABI::getArrayCookieSizeImpl(QualType type) {
  // The array cookie is always a size_t; we then pad that out to the
  // alignment of the element type.
  ASTContext &Ctx = getContext();
  return std::max(Ctx.getTypeSizeInChars(Ctx.getSizeType()),
                  Ctx.getTypeAlignInChars(type));
}

llvm::Value *MicrosoftFortranABI::readArrayCookieImpl(CodeGenFunction &CGF,
                                                  llvm::Value *allocPtr,
                                                  CharUnits cookieSize) {
  unsigned AS = allocPtr->getType()->getPointerAddressSpace();
  llvm::Value *numElementsPtr =
    CGF.Builder.CreateBitCast(allocPtr, CGF.SizeTy->getPointerTo(AS));
  return CGF.Builder.CreateLoad(numElementsPtr);
}

llvm::Value* MicrosoftFortranABI::InitializeArrayCookie(CodeGenFunction &CGF,
                                                    llvm::Value *newPtr,
                                                    llvm::Value *numElements,
                                                    const CXXNewExpr *expr,
                                                    QualType elementType) {
  assert(requiresArrayCookie(expr));

  // The size of the cookie.
  CharUnits cookieSize = getArrayCookieSizeImpl(elementType);

  // Compute an offset to the cookie.
  llvm::Value *cookiePtr = newPtr;

  // Write the number of elements into the appropriate slot.
  unsigned AS = newPtr->getType()->getPointerAddressSpace();
  llvm::Value *numElementsPtr
    = CGF.Builder.CreateBitCast(cookiePtr, CGF.SizeTy->getPointerTo(AS));
  CGF.Builder.CreateStore(numElements, numElementsPtr);

  // Finally, compute a pointer to the actual data buffer by skipping
  // over the cookie completely.
  return CGF.Builder.CreateConstInBoundsGEP1_64(newPtr,
                                                cookieSize.getQuantity());
}

void MicrosoftFortranABI::EmitGuardedInit(CodeGenFunction &CGF, const VarDecl &D,
                                      llvm::GlobalVariable *DeclPtr,
                                      bool PerformInit) {
  // FIXME: this code was only tested for global initialization.
  // Not sure whether we want thread-safe static local variables as VS
  // doesn't make them thread-safe.

  // Emit the initializer and add a global destructor if appropriate.
  CGF.EmitCXXGlobalVarDeclInit(D, DeclPtr, PerformInit);
}

void MicrosoftFortranABI::EmitVTables(const CXXRecordDecl *Class) {
  // FIXME: implement
}

CGFortranABI *lfort::CodeGen::CreateMicrosoftFortranABI(CodeGenModule &CGM) {
  return new MicrosoftFortranABI(CGM);
}

