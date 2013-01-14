//===--- CodeGenTypes.h - Type translation for LLVM CodeGen -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is the code that handles AST -> LLVM type lowering.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_CODEGEN_CODEGENTYPES_H
#define LFORT_CODEGEN_CODEGENTYPES_H

#include "CGCall.h"
#include "lfort/AST/GlobalDecl.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/Module.h"
#include <vector>

namespace llvm {
  class SubprogramType;
  class Module;
  class DataLayout;
  class Type;
  class LLVMContext;
  class StructType;
}

namespace lfort {
  class ABIInfo;
  class ASTContext;
  template <typename> class CanQual;
  class CXXConstructorDecl;
  class CXXDestructorDecl;
  class CXXMethodDecl;
  class CodeGenOptions;
  class FieldDecl;
  class SubprogramProtoType;
  class ObjCInterfaceDecl;
  class ObjCIvarDecl;
  class PointerType;
  class QualType;
  class RecordDecl;
  class TagDecl;
  class TargetInfo;
  class Type;
  typedef CanQual<Type> CanQualType;

namespace CodeGen {
  class CGFortranABI;
  class CGRecordLayout;
  class CodeGenModule;
  class RequiredArgs;

/// CodeGenTypes - This class organizes the cross-module state that is used
/// while lowering AST types to LLVM types.
class CodeGenTypes {
public:
  // Some of this stuff should probably be left on the CGM.
  ASTContext &Context;
  const TargetInfo &Target;
  llvm::Module &TheModule;
  const llvm::DataLayout &TheDataLayout;
  const ABIInfo &TheABIInfo;
  CGFortranABI &TheFortranABI;
  const CodeGenOptions &CodeGenOpts;
  CodeGenModule &CGM;

private:
  /// The opaque type map for Objective-C interfaces. All direct
  /// manipulation is done by the runtime interfaces, which are
  /// responsible for coercing to the appropriate type; these opaque
  /// types are never refined.
  llvm::DenseMap<const ObjCInterfaceType*, llvm::Type *> InterfaceTypes;

  /// CGRecordLayouts - This maps llvm struct type with corresponding
  /// record layout info.
  llvm::DenseMap<const Type*, CGRecordLayout *> CGRecordLayouts;

  /// RecordDeclTypes - This contains the LLVM IR type for any converted
  /// RecordDecl.
  llvm::DenseMap<const Type*, llvm::StructType *> RecordDeclTypes;
  
  /// SubprogramInfos - Hold memoized CGSubprogramInfo results.
  llvm::FoldingSet<CGSubprogramInfo> SubprogramInfos;

  /// RecordsBeingLaidOut - This set keeps track of records that we're currently
  /// converting to an IR type.  For example, when converting:
  /// struct A { struct B { int x; } } when processing 'x', the 'A' and 'B'
  /// types will be in this set.
  llvm::SmallPtrSet<const Type*, 4> RecordsBeingLaidOut;
  
  llvm::SmallPtrSet<const CGSubprogramInfo*, 4> SubprogramsBeingProcessed;
  
  /// SkippedLayout - True if we didn't layout a function due to a being inside
  /// a recursive struct conversion, set this to true.
  bool SkippedLayout;

  SmallVector<const RecordDecl *, 8> DeferredRecords;
  
private:
  /// TypeCache - This map keeps cache of llvm::Types
  /// and maps llvm::Types to corresponding lfort::Type.
  llvm::DenseMap<const Type *, llvm::Type *> TypeCache;

public:
  CodeGenTypes(CodeGenModule &CGM);
  ~CodeGenTypes();

  const llvm::DataLayout &getDataLayout() const { return TheDataLayout; }
  const TargetInfo &getTarget() const { return Target; }
  ASTContext &getContext() const { return Context; }
  const ABIInfo &getABIInfo() const { return TheABIInfo; }
  const CodeGenOptions &getCodeGenOpts() const { return CodeGenOpts; }
  CGFortranABI &getFortranABI() const { return TheFortranABI; }
  llvm::LLVMContext &getLLVMContext() { return TheModule.getContext(); }

  /// ConvertType - Convert type T into a llvm::Type.
  llvm::Type *ConvertType(QualType T);

  /// ConvertTypeForMem - Convert type T into a llvm::Type.  This differs from
  /// ConvertType in that it is used to convert to the memory representation for
  /// a type.  For example, the scalar representation for _Bool is i1, but the
  /// memory representation is usually i8 or i32, depending on the target.
  llvm::Type *ConvertTypeForMem(QualType T);

  /// GetSubprogramType - Get the LLVM function type for \arg Info.
  llvm::FunctionType *GetSubprogramType(const CGSubprogramInfo &Info);

  llvm::FunctionType *GetSubprogramType(GlobalDecl GD);

  /// isFuncTypeConvertible - Utility to check whether a function type can
  /// be converted to an LLVM type (i.e. doesn't depend on an incomplete tag
  /// type).
  bool isFuncTypeConvertible(const SubprogramType *FT);
  bool isFuncTypeArgumentConvertible(QualType Ty);
  
  /// GetSubprogramTypeForVTable - Get the LLVM function type for use in a vtable,
  /// given a CXXMethodDecl. If the method to has an incomplete return type,
  /// and/or incomplete argument types, this will return the opaque type.
  llvm::Type *GetSubprogramTypeForVTable(GlobalDecl GD);

  const CGRecordLayout &getCGRecordLayout(const RecordDecl*);

  /// UpdateCompletedType - When we find the full definition for a TagDecl,
  /// replace the 'opaque' type we previously made for it if applicable.
  void UpdateCompletedType(const TagDecl *TD);

  /// getNullarySubprogramInfo - Get the function info for a void()
  /// function with standard CC.
  const CGSubprogramInfo &arrangeNullarySubprogram();

  // The arrangement methods are split into three families:
  //   - those meant to drive the signature and prologue/epilogue
  //     of a function declaration or definition,
  //   - those meant for the computation of the LLVM type for an abstract
  //     appearance of a function, and
  //   - those meant for performing the IR-generation of a call.
  // They differ mainly in how they deal with optional (i.e. variadic)
  // arguments, as well as unprototyped functions.
  //
  // Key points:
  // - The CGSubprogramInfo for emitting a specific call site must include
  //   entries for the optional arguments.
  // - The function type used at the call site must reflect the formal
  //   signature of the declaration being called, or else the call will
  //   go awry.
  // - For the most part, unprototyped functions are called by casting to
  //   a formal signature inferred from the specific argument types used
  //   at the call-site.  However, some targets (e.g. x86-64) screw with
  //   this for compatibility reasons.

  const CGSubprogramInfo &arrangeGlobalDeclaration(GlobalDecl GD);
  const CGSubprogramInfo &arrangeSubprogramDeclaration(const SubprogramDecl *FD);
  const CGSubprogramInfo &arrangeSubprogramDeclaration(QualType ResTy,
                                                   const SubprogramArgList &Args,
                                             const SubprogramType::ExtInfo &Info,
                                                   bool isVariadic);

  const CGSubprogramInfo &arrangeObjCMethodDeclaration(const ObjCMethodDecl *MD);
  const CGSubprogramInfo &arrangeObjCMessageSendSignature(const ObjCMethodDecl *MD,
                                                        QualType receiverType);

  const CGSubprogramInfo &arrangeCXXMethodDeclaration(const CXXMethodDecl *MD);
  const CGSubprogramInfo &arrangeCXXConstructorDeclaration(
                                                    const CXXConstructorDecl *D,
                                                    CXXCtorType Type);
  const CGSubprogramInfo &arrangeCXXDestructor(const CXXDestructorDecl *D,
                                             CXXDtorType Type);

  const CGSubprogramInfo &arrangeFreeSubprogramCall(const CallArgList &Args,
                                                const SubprogramType *Ty);
  const CGSubprogramInfo &arrangeFreeSubprogramCall(QualType ResTy,
                                                const CallArgList &args,
                                                SubprogramType::ExtInfo info,
                                                RequiredArgs required);
  const CGSubprogramInfo &arrangeBlockSubprogramCall(const CallArgList &args,
                                                 const SubprogramType *type);

  const CGSubprogramInfo &arrangeCXXMethodCall(const CallArgList &args,
                                             const SubprogramProtoType *type,
                                             RequiredArgs required);

  const CGSubprogramInfo &arrangeFreeSubprogramType(CanQual<SubprogramProtoType> Ty);
  const CGSubprogramInfo &arrangeFreeSubprogramType(CanQual<SubprogramNoProtoType> Ty);
  const CGSubprogramInfo &arrangeCXXMethodType(const CXXRecordDecl *RD,
                                             const SubprogramProtoType *FTP);

  /// "Arrange" the LLVM information for a call or type with the given
  /// signature.  This is largely an internal method; other clients
  /// should use one of the above routines, which ultimately defer to
  /// this.
  ///
  /// \param argTypes - must all actually be canonical as params
  const CGSubprogramInfo &arrangeLLVMSubprogramInfo(CanQualType returnType,
                                                ArrayRef<CanQualType> argTypes,
                                                SubprogramType::ExtInfo info,
                                                RequiredArgs args);

  /// \brief Compute a new LLVM record layout object for the given record.
  CGRecordLayout *ComputeRecordLayout(const RecordDecl *D,
                                      llvm::StructType *Ty);

  /// addRecordTypeName - Compute a name from the given record decl with an
  /// optional suffix and name the given LLVM type using it.
  void addRecordTypeName(const RecordDecl *RD, llvm::StructType *Ty,
                         StringRef suffix);
  

public:  // These are internal details of CGT that shouldn't be used externally.
  /// ConvertRecordDeclType - Lay out a tagged decl type like struct or union.
  llvm::StructType *ConvertRecordDeclType(const RecordDecl *TD);

  /// GetExpandedTypes - Expand the type \arg Ty into the LLVM
  /// argument types it would be passed as on the provided vector \arg
  /// ArgTys. See ABIArgInfo::Expand.
  void GetExpandedTypes(QualType type,
                        SmallVectorImpl<llvm::Type*> &expanded);

  /// IsZeroInitializable - Return whether a type can be
  /// zero-initialized (in the C++ sense) with an LLVM zeroinitializer.
  bool isZeroInitializable(QualType T);

  /// IsZeroInitializable - Return whether a record type can be
  /// zero-initialized (in the C++ sense) with an LLVM zeroinitializer.
  bool isZeroInitializable(const CXXRecordDecl *RD);
  
  bool isRecordLayoutComplete(const Type *Ty) const;
  bool noRecordsBeingLaidOut() const {
    return RecordsBeingLaidOut.empty();
  }
  bool isRecordBeingLaidOut(const Type *Ty) const {
    return RecordsBeingLaidOut.count(Ty);
  }
                            
};

}  // end namespace CodeGen
}  // end namespace lfort

#endif
