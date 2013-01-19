//===--- CGDeclCXX.cpp - Emit LLVM Code for C++ declarations --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This contains code dealing with code generation of C++ declarations
//
//===----------------------------------------------------------------------===//

#include "CodeGenSubprogram.h"
#include "CGFortranABI.h"
#include "CGObjCRuntime.h"
#include "lfort/Frontend/CodeGenOptions.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/IR/Intrinsics.h"

using namespace lfort;
using namespace CodeGen;

static void EmitDeclInit(CodeGenSubprogram &CGF, const VarDecl &D,
                         llvm::Constant *DeclPtr) {
  assert(D.hasGlobalStorage() && "VarDecl must have global storage!");
  assert(!D.getType()->isReferenceType() && 
         "Should not call EmitDeclInit on a reference!");
  
  ASTContext &Context = CGF.getContext();

  CharUnits alignment = Context.getDeclAlign(&D);
  QualType type = D.getType();
  LValue lv = CGF.MakeAddrLValue(DeclPtr, type, alignment);

  const Expr *Init = D.getInit();
  if (!CGF.hasAggregateLLVMType(type)) {
    CodeGenModule &CGM = CGF.CGM;
    if (lv.isObjCStrong())
      CGM.getObjCRuntime().EmitObjCGlobalAssign(CGF, CGF.EmitScalarExpr(Init),
                                                DeclPtr, D.isThreadSpecified());
    else if (lv.isObjCWeak())
      CGM.getObjCRuntime().EmitObjCWeakAssign(CGF, CGF.EmitScalarExpr(Init),
                                              DeclPtr);
    else
      CGF.EmitScalarInit(Init, &D, lv, false);
  } else if (type->isAnyComplexType()) {
    CGF.EmitComplexExprIntoAddr(Init, DeclPtr, lv.isVolatile());
  } else {
    CGF.EmitAggExpr(Init, AggValueSlot::forLValue(lv,AggValueSlot::IsDestructed,
                                          AggValueSlot::DoesNotNeedGCBarriers,
                                                  AggValueSlot::IsNotAliased));
  }
}

/// Emit code to cause the destruction of the given variable with
/// static storage duration.
static void EmitDeclDestroy(CodeGenSubprogram &CGF, const VarDecl &D,
                            llvm::Constant *addr) {
  CodeGenModule &CGM = CGF.CGM;

  // FIXME:  __attribute__((cleanup)) ?
  
  QualType type = D.getType();
  QualType::DestructionKind dtorKind = type.isDestructedType();

  switch (dtorKind) {
  case QualType::DK_none:
    return;

  case QualType::DK_cxx_destructor:
    break;

  case QualType::DK_objc_strong_lifetime:
  case QualType::DK_objc_weak_lifetime:
    // We don't care about releasing objects during process teardown.
    return;
  }

  llvm::Constant *function;
  llvm::Constant *argument;

  // Special-case non-array C++ destructors, where there's a function
  // with the right signature that we can just call.
  const CXXRecordDecl *record = 0;
  if (dtorKind == QualType::DK_cxx_destructor &&
      (record = type->getAsCXXRecordDecl())) {
    assert(!record->hasTrivialDestructor());
    CXXDestructorDecl *dtor = record->getDestructor();

    function = CGM.GetAddrOfCXXDestructor(dtor, Dtor_Complete);
    argument = addr;

  // Otherwise, the standard logic requires a helper function.
  } else {
    function = CodeGenSubprogram(CGM).generateDestroyHelper(addr, type,
                                                  CGF.getDestroyer(dtorKind),
                                                  CGF.needsEHCleanup(dtorKind));
    argument = llvm::Constant::getNullValue(CGF.Int8PtrTy);
  }

  CGM.getFortranABI().registerGlobalDtor(CGF, function, argument);
}

/// Emit code to cause the variable at the given address to be considered as
/// constant from this point onwards.
static void EmitDeclInvariant(CodeGenSubprogram &CGF, const VarDecl &D,
                              llvm::Constant *Addr) {
  // Don't emit the intrinsic if we're not optimizing.
  if (!CGF.CGM.getCodeGenOpts().OptimizationLevel)
    return;

  // Grab the llvm.invariant.start intrinsic.
  llvm::Intrinsic::ID InvStartID = llvm::Intrinsic::invariant_start;
  llvm::Constant *InvariantStart = CGF.CGM.getIntrinsic(InvStartID);

  // Emit a call with the size in bytes of the object.
  CharUnits WidthChars = CGF.getContext().getTypeSizeInChars(D.getType());
  uint64_t Width = WidthChars.getQuantity();
  llvm::Value *Args[2] = { llvm::ConstantInt::getSigned(CGF.Int64Ty, Width),
                           llvm::ConstantExpr::getBitCast(Addr, CGF.Int8PtrTy)};
  CGF.Builder.CreateCall(InvariantStart, Args);
}

void CodeGenSubprogram::EmitCXXGlobalVarDeclInit(const VarDecl &D,
                                               llvm::Constant *DeclPtr,
                                               bool PerformInit) {

  const Expr *Init = D.getInit();
  QualType T = D.getType();

  if (!T->isReferenceType()) {
    if (PerformInit)
      EmitDeclInit(*this, D, DeclPtr);
    if (CGM.isTypeConstant(D.getType(), true))
      EmitDeclInvariant(*this, D, DeclPtr);
    else
      EmitDeclDestroy(*this, D, DeclPtr);
    return;
  }

  assert(PerformInit && "cannot have constant initializer which needs "
         "destruction for reference");
  unsigned Alignment = getContext().getDeclAlign(&D).getQuantity();
  RValue RV = EmitReferenceBindingToExpr(Init, &D);
  EmitStoreOfScalar(RV.getScalarVal(), DeclPtr, false, Alignment, T);
}

static llvm::Function *
CreateGlobalInitOrDestructSubprogram(CodeGenModule &CGM,
                                   llvm::FunctionType *ty,
                                   const Twine &name);

/// Create a stub function, suitable for being passed to atexit,
/// which passes the given address to the given destructor function.
static llvm::Constant *createAtExitStub(CodeGenModule &CGM,
                                        llvm::Constant *dtor,
                                        llvm::Constant *addr) {
  // Get the destructor function type, void(*)(void).
  llvm::FunctionType *ty = llvm::FunctionType::get(CGM.VoidTy, false);
  llvm::Function *fn =
    CreateGlobalInitOrDestructSubprogram(CGM, ty,
                                       Twine("__dtor_", addr->getName()));

  CodeGenSubprogram CGF(CGM);

  // Initialize debug info if needed.
  CGF.maybeInitializeDebugInfo();

  CGF.StartSubprogram(GlobalDecl(), CGM.getContext().VoidTy, fn,
                    CGM.getTypes().arrangeNullarySubprogram(),
                    SubprogramArgList(), SourceLocation());

  llvm::CallInst *call = CGF.Builder.CreateCall(dtor, addr);
 
 // Make sure the call and the callee agree on calling convention.
  if (llvm::Function *dtorFn =
        dyn_cast<llvm::Function>(dtor->stripPointerCasts()))
    call->setCallingConv(dtorFn->getCallingConv());

  CGF.FinishSubprogram();

  return fn;
}

/// Register a global destructor using the C atexit runtime function.
void CodeGenSubprogram::registerGlobalDtorWithAtExit(llvm::Constant *dtor,
                                                   llvm::Constant *addr) {
  // Create a function which calls the destructor.
  llvm::Constant *dtorStub = createAtExitStub(CGM, dtor, addr);

  // extern "C" int atexit(void (*f)(void));
  llvm::FunctionType *atexitTy =
    llvm::FunctionType::get(IntTy, dtorStub->getType(), false);

  llvm::Constant *atexit =
    CGM.CreateRuntimeSubprogram(atexitTy, "atexit");
  if (llvm::Function *atexitFn = dyn_cast<llvm::Function>(atexit))
    atexitFn->setDoesNotThrow();

  Builder.CreateCall(atexit, dtorStub)->setDoesNotThrow();
}

void CodeGenSubprogram::EmitCXXGuardedInit(const VarDecl &D,
                                         llvm::GlobalVariable *DeclPtr,
                                         bool PerformInit) {
  // If we've been asked to forbid guard variables, emit an error now.
  // This diagnostic is hard-coded for Darwin's use case;  we can find
  // better phrasing if someone else needs it.
  if (CGM.getCodeGenOpts().ForbidGuardVariables)
    CGM.Error(D.getLocation(),
              "this initialization requires a guard variable, which "
              "the kernel does not support");

  CGM.getFortranABI().EmitGuardedInit(*this, D, DeclPtr, PerformInit);
}

static llvm::Function *
CreateGlobalInitOrDestructSubprogram(CodeGenModule &CGM,
                                   llvm::FunctionType *FTy,
                                   const Twine &Name) {
  llvm::Function *Fn =
    llvm::Function::Create(FTy, llvm::GlobalValue::InternalLinkage,
                           Name, &CGM.getModule());
  if (!CGM.getLangOpts().AppleKext) {
    // Set the section if needed.
    if (const char *Section = 
          CGM.getContext().getTargetInfo().getStaticInitSectionSpecifier())
      Fn->setSection(Section);
  }

  if (!CGM.getLangOpts().Exceptions)
    Fn->setDoesNotThrow();

  if (CGM.getLangOpts().SanitizeAddress)
    Fn->addFnAttr(llvm::Attribute::AddressSafety);

  return Fn;
}

void
CodeGenModule::EmitCXXGlobalVarDeclInitFunc(const VarDecl *D,
                                            llvm::GlobalVariable *Addr,
                                            bool PerformInit) {
  llvm::FunctionType *FTy = llvm::FunctionType::get(VoidTy, false);

  // Create a variable initialization function.
  llvm::Function *Fn =
    CreateGlobalInitOrDestructSubprogram(*this, FTy, "__cxx_global_var_init");

  CodeGenSubprogram(*this).GenerateCXXGlobalVarDeclInitFunc(Fn, D, Addr,
                                                          PerformInit);

  if (D->hasAttr<InitPriorityAttr>()) {
    unsigned int order = D->getAttr<InitPriorityAttr>()->getPriority();
    OrderGlobalInits Key(order, PrioritizedCXXGlobalInits.size());
    PrioritizedCXXGlobalInits.push_back(std::make_pair(Key, Fn));
    DelayedCXXInitPosition.erase(D);
  }  else {
    llvm::DenseMap<const Decl *, unsigned>::iterator I =
      DelayedCXXInitPosition.find(D);
    if (I == DelayedCXXInitPosition.end()) {
      CXXGlobalInits.push_back(Fn);
    } else {
      assert(CXXGlobalInits[I->second] == 0);
      CXXGlobalInits[I->second] = Fn;
      DelayedCXXInitPosition.erase(I);
    }
  }
}

void
CodeGenModule::EmitCXXGlobalInitFunc() {
  while (!CXXGlobalInits.empty() && !CXXGlobalInits.back())
    CXXGlobalInits.pop_back();

  if (CXXGlobalInits.empty() && PrioritizedCXXGlobalInits.empty())
    return;

  llvm::FunctionType *FTy = llvm::FunctionType::get(VoidTy, false);


  // Create our global initialization function.
  if (!PrioritizedCXXGlobalInits.empty()) {
    SmallVector<llvm::Constant*, 8> LocalCXXGlobalInits;
    llvm::array_pod_sort(PrioritizedCXXGlobalInits.begin(), 
                         PrioritizedCXXGlobalInits.end());
    // Iterate over "chunks" of ctors with same priority and emit each chunk
    // into separate function. Note - everything is sorted first by priority,
    // second - by lex order, so we emit ctor functions in proper order.
    for (SmallVectorImpl<GlobalInitData >::iterator
           I = PrioritizedCXXGlobalInits.begin(),
           E = PrioritizedCXXGlobalInits.end(); I != E; ) {
      SmallVectorImpl<GlobalInitData >::iterator
        PrioE = std::upper_bound(I + 1, E, *I, GlobalInitPriorityCmp());

      LocalCXXGlobalInits.clear();
      unsigned Priority = I->first.priority;
      // Compute the function suffix from priority. Prepend with zeroes to make
      // sure the function names are also ordered as priorities.
      std::string PrioritySuffix = llvm::utostr(Priority);
      // Priority is always <= 65535 (enforced by sema)..
      PrioritySuffix = std::string(6-PrioritySuffix.size(), '0')+PrioritySuffix;
      llvm::Function *Fn = 
        CreateGlobalInitOrDestructSubprogram(*this, FTy,
                                           "_GLOBAL__I_" + PrioritySuffix);
      
      for (; I < PrioE; ++I)
        LocalCXXGlobalInits.push_back(I->second);

      CodeGenSubprogram(*this).GenerateCXXGlobalInitFunc(Fn,
                                                    &LocalCXXGlobalInits[0],
                                                    LocalCXXGlobalInits.size());
      AddGlobalCtor(Fn, Priority);
    }
  }
  
  llvm::Function *Fn = 
    CreateGlobalInitOrDestructSubprogram(*this, FTy, "_GLOBAL__I_a");

  CodeGenSubprogram(*this).GenerateCXXGlobalInitFunc(Fn,
                                                   &CXXGlobalInits[0],
                                                   CXXGlobalInits.size());
  AddGlobalCtor(Fn);

  CXXGlobalInits.clear();
  PrioritizedCXXGlobalInits.clear();
}

void CodeGenModule::EmitCXXGlobalDtorFunc() {
  if (CXXGlobalDtors.empty())
    return;

  llvm::FunctionType *FTy = llvm::FunctionType::get(VoidTy, false);

  // Create our global destructor function.
  llvm::Function *Fn =
    CreateGlobalInitOrDestructSubprogram(*this, FTy, "_GLOBAL__D_a");

  CodeGenSubprogram(*this).GenerateCXXGlobalDtorsFunc(Fn, CXXGlobalDtors);
  AddGlobalDtor(Fn);
}

/// Emit the code necessary to initialize the given global variable.
void CodeGenSubprogram::GenerateCXXGlobalVarDeclInitFunc(llvm::Function *Fn,
                                                       const VarDecl *D,
                                                 llvm::GlobalVariable *Addr,
                                                       bool PerformInit) {
  // Check if we need to emit debug info for variable initializer.
  if (!D->hasAttr<NoDebugAttr>())
    maybeInitializeDebugInfo();

  StartSubprogram(GlobalDecl(D), getContext().VoidTy, Fn,
                getTypes().arrangeNullarySubprogram(),
                SubprogramArgList(), D->getInit()->getExprLoc());

  // Use guarded initialization if the global variable is weak. This
  // occurs for, e.g., instantiated static data members and
  // definitions explicitly marked weak.
  if (Addr->getLinkage() == llvm::GlobalValue::WeakODRLinkage ||
      Addr->getLinkage() == llvm::GlobalValue::WeakAnyLinkage) {
    EmitCXXGuardedInit(*D, Addr, PerformInit);
  } else {
    EmitCXXGlobalVarDeclInit(*D, Addr, PerformInit);
  }

  FinishSubprogram();
}

void CodeGenSubprogram::GenerateCXXGlobalInitFunc(llvm::Function *Fn,
                                                llvm::Constant **Decls,
                                                unsigned NumDecls) {
  // Initialize debug info if needed.
  maybeInitializeDebugInfo();

  StartSubprogram(GlobalDecl(), getContext().VoidTy, Fn,
                getTypes().arrangeNullarySubprogram(),
                SubprogramArgList(), SourceLocation());

  RunCleanupsScope Scope(*this);

  // When building in Objective-C++ ARC mode, create an autorelease pool
  // around the global initializers.
  if (getLangOpts().ObjCAutoRefCount && getLangOpts().F90) {    
    llvm::Value *token = EmitObjCAutoreleasePoolPush();
    EmitObjCAutoreleasePoolCleanup(token);
  }
  
  for (unsigned i = 0; i != NumDecls; ++i)
    if (Decls[i])
      Builder.CreateCall(Decls[i]);    

  Scope.ForceCleanup();
  
  FinishSubprogram();
}

void CodeGenSubprogram::GenerateCXXGlobalDtorsFunc(llvm::Function *Fn,
                  const std::vector<std::pair<llvm::WeakVH, llvm::Constant*> >
                                                &DtorsAndObjects) {
  // Initialize debug info if needed.
  maybeInitializeDebugInfo();

  StartSubprogram(GlobalDecl(), getContext().VoidTy, Fn,
                getTypes().arrangeNullarySubprogram(),
                SubprogramArgList(), SourceLocation());

  // Emit the dtors, in reverse order from construction.
  for (unsigned i = 0, e = DtorsAndObjects.size(); i != e; ++i) {
    llvm::Value *Callee = DtorsAndObjects[e - i - 1].first;
    llvm::CallInst *CI = Builder.CreateCall(Callee,
                                            DtorsAndObjects[e - i - 1].second);
    // Make sure the call and the callee agree on calling convention.
    if (llvm::Function *F = dyn_cast<llvm::Function>(Callee))
      CI->setCallingConv(F->getCallingConv());
  }

  FinishSubprogram();
}

/// generateDestroyHelper - Generates a helper function which, when
/// invoked, destroys the given object.
llvm::Function * 
CodeGenSubprogram::generateDestroyHelper(llvm::Constant *addr,
                                       QualType type,
                                       Destroyer *destroyer,
                                       bool useEHCleanupForArray) {
  SubprogramArgList args;
  ImplicitParamDecl dst(0, SourceLocation(), 0, getContext().VoidPtrTy);
  args.push_back(&dst);
  
  const CGSubprogramInfo &FI = 
    CGM.getTypes().arrangeSubprogramDeclaration(getContext().VoidTy, args,
                                              SubprogramType::ExtInfo(),
                                              /*variadic*/ false);
  llvm::FunctionType *FTy = CGM.getTypes().GetSubprogramType(FI);
  llvm::Function *fn = 
    CreateGlobalInitOrDestructSubprogram(CGM, FTy, "__cxx_global_array_dtor");

  // Initialize debug info if needed.
  maybeInitializeDebugInfo();

  StartSubprogram(GlobalDecl(), getContext().VoidTy, fn, FI, args,
                SourceLocation());

  emitDestroy(addr, type, destroyer, useEHCleanupForArray);
  
  FinishSubprogram();
  
  return fn;
}
