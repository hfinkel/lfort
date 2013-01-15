//===- CXTypes.cpp - Implements 'CXTypes' aspect of liblfort ------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===--------------------------------------------------------------------===//
//
// This file implements the 'CXTypes' API hooks in the LFort-C library.
//
//===--------------------------------------------------------------------===//

#include "CIndexer.h"
#include "CXCursor.h"
#include "CXString.h"
#include "CXProgram.h"
#include "CXType.h"
#include "lfort/AST/Decl.h"
#include "lfort/AST/DeclObjC.h"
#include "lfort/AST/DeclTemplate.h"
#include "lfort/AST/Expr.h"
#include "lfort/AST/Type.h"
#include "lfort/Frontend/ASTUnit.h"

using namespace lfort;

static CXTypeKind GetBuiltinTypeKind(const BuiltinType *BT) {
#define BTCASE(K) case BuiltinType::K: return CXType_##K
  switch (BT->getKind()) {
    BTCASE(Void);
    BTCASE(Bool);
    BTCASE(Char_U);
    BTCASE(UChar);
    BTCASE(Char16);
    BTCASE(Char32);
    BTCASE(UShort);
    BTCASE(UInt);
    BTCASE(ULong);
    BTCASE(ULongLong);
    BTCASE(UInt128);
    BTCASE(Char_S);
    BTCASE(SChar);
    case BuiltinType::WChar_S: return CXType_WChar;
    case BuiltinType::WChar_U: return CXType_WChar;
    BTCASE(Short);
    BTCASE(Int);
    BTCASE(Long);
    BTCASE(LongLong);
    BTCASE(Int128);
    BTCASE(Float);
    BTCASE(Double);
    BTCASE(LongDouble);
    BTCASE(NullPtr);
    BTCASE(Overload);
    BTCASE(Dependent);
    BTCASE(ObjCId);
    BTCASE(ObjCClass);
    BTCASE(ObjCSel);
  default:
    return CXType_Unexposed;
  }
#undef BTCASE
}

static CXTypeKind GetTypeKind(QualType T) {
  const Type *TP = T.getTypePtrOrNull();
  if (!TP)
    return CXType_Invalid;

#define TKCASE(K) case Type::K: return CXType_##K
  switch (TP->getTypeClass()) {
    case Type::Builtin:
      return GetBuiltinTypeKind(cast<BuiltinType>(TP));
    TKCASE(Complex);
    TKCASE(Pointer);
    TKCASE(BlockPointer);
    TKCASE(LValueReference);
    TKCASE(RValueReference);
    TKCASE(Record);
    TKCASE(Enum);
    TKCASE(Typedef);
    TKCASE(ObjCInterface);
    TKCASE(ObjCObjectPointer);
    TKCASE(SubprogramNoProto);
    TKCASE(SubprogramProto);
    TKCASE(ConstantArray);
    TKCASE(Vector);
    default:
      return CXType_Unexposed;
  }
#undef TKCASE
}


CXType cxtype::MakeCXType(QualType T, CXProgram Pgm) {
  CXTypeKind TK = CXType_Invalid;

  if (Pgm && !T.isNull()) {
    ASTContext &Ctx = static_cast<ASTUnit *>(Pgm->PgmData)->getASTContext();
    if (Ctx.getLangOpts().ObjC1) {
      QualType UnqualT = T.getUnqualifiedType();
      if (Ctx.isObjCIdType(UnqualT))
        TK = CXType_ObjCId;
      else if (Ctx.isObjCClassType(UnqualT))
        TK = CXType_ObjCClass;
      else if (Ctx.isObjCSelType(UnqualT))
        TK = CXType_ObjCSel;
    }
  }
  if (TK == CXType_Invalid)
    TK = GetTypeKind(T);

  CXType CT = { TK, { TK == CXType_Invalid ? 0 : T.getAsOpaquePtr(), Pgm }};
  return CT;
}

using cxtype::MakeCXType;

static inline QualType GetQualType(CXType CT) {
  return QualType::getFromOpaquePtr(CT.data[0]);
}

static inline CXProgram GetPgm(CXType CT) {
  return static_cast<CXProgram>(CT.data[1]);
}

extern "C" {

CXType lfort_getCursorType(CXCursor C) {
  using namespace cxcursor;
  
  CXProgram Pgm = cxcursor::getCursorPgm(C);
  ASTContext &Context = static_cast<ASTUnit *>(Pgm->PgmData)->getASTContext();
  if (lfort_isExpression(C.kind)) {
    QualType T = cxcursor::getCursorExpr(C)->getType();
    return MakeCXType(T, Pgm);
  }

  if (lfort_isDeclaration(C.kind)) {
    Decl *D = cxcursor::getCursorDecl(C);
    if (!D)
      return MakeCXType(QualType(), Pgm);

    if (TypeDecl *TD = dyn_cast<TypeDecl>(D))
      return MakeCXType(Context.getTypeDeclType(TD), Pgm);
    if (ObjCInterfaceDecl *ID = dyn_cast<ObjCInterfaceDecl>(D))
      return MakeCXType(Context.getObjCInterfaceType(ID), Pgm);
    if (ValueDecl *VD = dyn_cast<ValueDecl>(D))
      return MakeCXType(VD->getType(), Pgm);
    if (ObjCPropertyDecl *PD = dyn_cast<ObjCPropertyDecl>(D))
      return MakeCXType(PD->getType(), Pgm);
    if (SubprogramDecl *FD = dyn_cast<SubprogramDecl>(D))
      return MakeCXType(FD->getType(), Pgm);
    return MakeCXType(QualType(), Pgm);
  }
  
  if (lfort_isReference(C.kind)) {
    switch (C.kind) {
    case CXCursor_ObjCSuperClassRef: {
      QualType T
        = Context.getObjCInterfaceType(getCursorObjCSuperClassRef(C).first);
      return MakeCXType(T, Pgm);
    }
        
    case CXCursor_ObjCClassRef: {
      QualType T = Context.getObjCInterfaceType(getCursorObjCClassRef(C).first);
      return MakeCXType(T, Pgm);
    }
        
    case CXCursor_TypeRef: {
      QualType T = Context.getTypeDeclType(getCursorTypeRef(C).first);
      return MakeCXType(T, Pgm);

    }
      
    case CXCursor_CXXBaseSpecifier:
      return cxtype::MakeCXType(getCursorCXXBaseSpecifier(C)->getType(), Pgm);

    case CXCursor_MemberRef:
      return cxtype::MakeCXType(getCursorMemberRef(C).first->getType(), Pgm);

    case CXCursor_VariableRef:
      return cxtype::MakeCXType(getCursorVariableRef(C).first->getType(), Pgm);

    case CXCursor_ObjCProtocolRef:
    case CXCursor_TemplateRef:
    case CXCursor_NamespaceRef:
    case CXCursor_OverloadedDeclRef:
    default:
      break;
    }
    
    return MakeCXType(QualType(), Pgm);
  }

  return MakeCXType(QualType(), Pgm);
}

CXType lfort_getTypedefDeclUnderlyingType(CXCursor C) {
  using namespace cxcursor;
  CXProgram Pgm = cxcursor::getCursorPgm(C);

  if (lfort_isDeclaration(C.kind)) {
    Decl *D = cxcursor::getCursorDecl(C);

    if (TypedefNameDecl *TD = dyn_cast_or_null<TypedefNameDecl>(D)) {
      QualType T = TD->getUnderlyingType();
      return MakeCXType(T, Pgm);
    }

    return MakeCXType(QualType(), Pgm);
  }

  return MakeCXType(QualType(), Pgm);
}

CXType lfort_getEnumDeclIntegerType(CXCursor C) {
  using namespace cxcursor;
  CXProgram Pgm = cxcursor::getCursorPgm(C);

  if (lfort_isDeclaration(C.kind)) {
    Decl *D = cxcursor::getCursorDecl(C);

    if (EnumDecl *TD = dyn_cast_or_null<EnumDecl>(D)) {
      QualType T = TD->getIntegerType();
      return MakeCXType(T, Pgm);
    }

    return MakeCXType(QualType(), Pgm);
  }

  return MakeCXType(QualType(), Pgm);
}

long long lfort_getEnumConstantDeclValue(CXCursor C) {
  using namespace cxcursor;

  if (lfort_isDeclaration(C.kind)) {
    Decl *D = cxcursor::getCursorDecl(C);

    if (EnumConstantDecl *TD = dyn_cast_or_null<EnumConstantDecl>(D)) {
      return TD->getInitVal().getSExtValue();
    }

    return LLONG_MIN;
  }

  return LLONG_MIN;
}

unsigned long long lfort_getEnumConstantDeclUnsignedValue(CXCursor C) {
  using namespace cxcursor;

  if (lfort_isDeclaration(C.kind)) {
    Decl *D = cxcursor::getCursorDecl(C);

    if (EnumConstantDecl *TD = dyn_cast_or_null<EnumConstantDecl>(D)) {
      return TD->getInitVal().getZExtValue();
    }

    return ULLONG_MAX;
  }

  return ULLONG_MAX;
}

int lfort_getFieldDeclBitWidth(CXCursor C) {
  using namespace cxcursor;

  if (lfort_isDeclaration(C.kind)) {
    Decl *D = getCursorDecl(C);

    if (FieldDecl *FD = dyn_cast_or_null<FieldDecl>(D)) {
      if (FD->isBitField())
        return FD->getBitWidthValue(getCursorContext(C));
    }
  }

  return -1;
}

CXType lfort_getCanonicalType(CXType CT) {
  if (CT.kind == CXType_Invalid)
    return CT;

  QualType T = GetQualType(CT);
  CXProgram Pgm = GetPgm(CT);

  if (T.isNull())
    return MakeCXType(QualType(), GetPgm(CT));

  ASTUnit *AU = static_cast<ASTUnit*>(Pgm->PgmData);
  return MakeCXType(AU->getASTContext().getCanonicalType(T), Pgm);
}

unsigned lfort_isConstQualifiedType(CXType CT) {
  QualType T = GetQualType(CT);
  return T.isLocalConstQualified();
}

unsigned lfort_isVolatileQualifiedType(CXType CT) {
  QualType T = GetQualType(CT);
  return T.isLocalVolatileQualified();
}

unsigned lfort_isRestrictQualifiedType(CXType CT) {
  QualType T = GetQualType(CT);
  return T.isLocalRestrictQualified();
}

CXType lfort_getPointeeType(CXType CT) {
  QualType T = GetQualType(CT);
  const Type *TP = T.getTypePtrOrNull();
  
  if (!TP)
    return MakeCXType(QualType(), GetPgm(CT));
  
  switch (TP->getTypeClass()) {
    case Type::Pointer:
      T = cast<PointerType>(TP)->getPointeeType();
      break;
    case Type::BlockPointer:
      T = cast<BlockPointerType>(TP)->getPointeeType();
      break;
    case Type::LValueReference:
    case Type::RValueReference:
      T = cast<ReferenceType>(TP)->getPointeeType();
      break;
    case Type::ObjCObjectPointer:
      T = cast<ObjCObjectPointerType>(TP)->getPointeeType();
      break;
    default:
      T = QualType();
      break;
  }
  return MakeCXType(T, GetPgm(CT));
}

CXCursor lfort_getTypeDeclaration(CXType CT) {
  if (CT.kind == CXType_Invalid)
    return cxcursor::MakeCXCursorInvalid(CXCursor_NoDeclFound);

  QualType T = GetQualType(CT);
  const Type *TP = T.getTypePtrOrNull();

  if (!TP)
    return cxcursor::MakeCXCursorInvalid(CXCursor_NoDeclFound);

  Decl *D = 0;

try_again:
  switch (TP->getTypeClass()) {
  case Type::Typedef:
    D = cast<TypedefType>(TP)->getDecl();
    break;
  case Type::ObjCObject:
    D = cast<ObjCObjectType>(TP)->getInterface();
    break;
  case Type::ObjCInterface:
    D = cast<ObjCInterfaceType>(TP)->getDecl();
    break;
  case Type::Record:
  case Type::Enum:
    D = cast<TagType>(TP)->getDecl();
    break;
  case Type::TemplateSpecialization:
    if (const RecordType *Record = TP->getAs<RecordType>())
      D = Record->getDecl();
    else
      D = cast<TemplateSpecializationType>(TP)->getTemplateName()
                                                         .getAsTemplateDecl();
    break;
      
  case Type::InjectedClassName:
    D = cast<InjectedClassNameType>(TP)->getDecl();
    break;

  // FIXME: Template type parameters!      

  case Type::Elaborated:
    TP = cast<ElaboratedType>(TP)->getNamedType().getTypePtrOrNull();
    goto try_again;
    
  default:
    break;
  }

  if (!D)
    return cxcursor::MakeCXCursorInvalid(CXCursor_NoDeclFound);

  return cxcursor::MakeCXCursor(D, GetPgm(CT));
}

CXString lfort_getTypeKindSpelling(enum CXTypeKind K) {
  const char *s = 0;
#define TKIND(X) case CXType_##X: s = ""  #X  ""; break
  switch (K) {
    TKIND(Invalid);
    TKIND(Unexposed);
    TKIND(Void);
    TKIND(Bool);
    TKIND(Char_U);
    TKIND(UChar);
    TKIND(Char16);
    TKIND(Char32);  
    TKIND(UShort);
    TKIND(UInt);
    TKIND(ULong);
    TKIND(ULongLong);
    TKIND(UInt128);
    TKIND(Char_S);
    TKIND(SChar);
    case CXType_WChar: s = "WChar"; break;
    TKIND(Short);
    TKIND(Int);
    TKIND(Long);
    TKIND(LongLong);
    TKIND(Int128);
    TKIND(Float);
    TKIND(Double);
    TKIND(LongDouble);
    TKIND(NullPtr);
    TKIND(Overload);
    TKIND(Dependent);
    TKIND(ObjCId);
    TKIND(ObjCClass);
    TKIND(ObjCSel);
    TKIND(Complex);
    TKIND(Pointer);
    TKIND(BlockPointer);
    TKIND(LValueReference);
    TKIND(RValueReference);
    TKIND(Record);
    TKIND(Enum);
    TKIND(Typedef);
    TKIND(ObjCInterface);
    TKIND(ObjCObjectPointer);
    TKIND(SubprogramNoProto);
    TKIND(SubprogramProto);
    TKIND(ConstantArray);
    TKIND(Vector);
  }
#undef TKIND
  return cxstring::createCXString(s);
}

unsigned lfort_equalTypes(CXType A, CXType B) {
  return A.data[0] == B.data[0] && A.data[1] == B.data[1];;
}

unsigned lfort_isSubprogramTypeVariadic(CXType X) {
  QualType T = GetQualType(X);
  if (T.isNull())
    return 0;

  if (const SubprogramProtoType *FD = T->getAs<SubprogramProtoType>())
    return (unsigned)FD->isVariadic();

  if (T->getAs<SubprogramNoProtoType>())
    return 1;
  
  return 0;
}

CXCallingConv lfort_getSubprogramTypeCallingConv(CXType X) {
  QualType T = GetQualType(X);
  if (T.isNull())
    return CXCallingConv_Invalid;
  
  if (const SubprogramType *FD = T->getAs<SubprogramType>()) {
#define TCALLINGCONV(X) case CC_##X: return CXCallingConv_##X
    switch (FD->getCallConv()) {
      TCALLINGCONV(Default);
      TCALLINGCONV(C);
      TCALLINGCONV(X86StdCall);
      TCALLINGCONV(X86FastCall);
      TCALLINGCONV(X86ThisCall);
      TCALLINGCONV(X86Pascal);
      TCALLINGCONV(AAPCS);
      TCALLINGCONV(AAPCS_VFP);
      TCALLINGCONV(PnaclCall);
      TCALLINGCONV(IntelOclBicc);
    }
#undef TCALLINGCONV
  }
  
  return CXCallingConv_Invalid;
}

int lfort_getNumArgTypes(CXType X) {
  QualType T = GetQualType(X);
  if (T.isNull())
    return -1;
  
  if (const SubprogramProtoType *FD = T->getAs<SubprogramProtoType>()) {
    return FD->getNumArgs();
  }
  
  if (T->getAs<SubprogramNoProtoType>()) {
    return 0;
  }
  
  return -1;
}

CXType lfort_getArgType(CXType X, unsigned i) {
  QualType T = GetQualType(X);
  if (T.isNull())
    return MakeCXType(QualType(), GetPgm(X));

  if (const SubprogramProtoType *FD = T->getAs<SubprogramProtoType>()) {
    unsigned numArgs = FD->getNumArgs();
    if (i >= numArgs)
      return MakeCXType(QualType(), GetPgm(X));
    
    return MakeCXType(FD->getArgType(i), GetPgm(X));
  }
  
  return MakeCXType(QualType(), GetPgm(X));
}

CXType lfort_getResultType(CXType X) {
  QualType T = GetQualType(X);
  if (T.isNull())
    return MakeCXType(QualType(), GetPgm(X));
  
  if (const SubprogramType *FD = T->getAs<SubprogramType>())
    return MakeCXType(FD->getResultType(), GetPgm(X));
  
  return MakeCXType(QualType(), GetPgm(X));
}

CXType lfort_getCursorResultType(CXCursor C) {
  if (lfort_isDeclaration(C.kind)) {
    Decl *D = cxcursor::getCursorDecl(C);
    if (const ObjCMethodDecl *MD = dyn_cast_or_null<ObjCMethodDecl>(D))
      return MakeCXType(MD->getResultType(), cxcursor::getCursorPgm(C));

    return lfort_getResultType(lfort_getCursorType(C));
  }

  return MakeCXType(QualType(), cxcursor::getCursorPgm(C));
}

unsigned lfort_isPODType(CXType X) {
  QualType T = GetQualType(X);
  if (T.isNull())
    return 0;
  
  CXProgram Pgm = GetPgm(X);
  ASTUnit *AU = static_cast<ASTUnit*>(Pgm->PgmData);

  return T.isPODType(AU->getASTContext()) ? 1 : 0;
}

CXType lfort_getElementType(CXType CT) {
  QualType ET = QualType();
  QualType T = GetQualType(CT);
  const Type *TP = T.getTypePtrOrNull();

  if (TP) {
    switch (TP->getTypeClass()) {
    case Type::ConstantArray:
      ET = cast<ConstantArrayType> (TP)->getElementType();
      break;
    case Type::Vector:
      ET = cast<VectorType> (TP)->getElementType();
      break;
    case Type::Complex:
      ET = cast<ComplexType> (TP)->getElementType();
      break;
    default:
      break;
    }
  }
  return MakeCXType(ET, GetPgm(CT));
}

long long lfort_getNumElements(CXType CT) {
  long long result = -1;
  QualType T = GetQualType(CT);
  const Type *TP = T.getTypePtrOrNull();

  if (TP) {
    switch (TP->getTypeClass()) {
    case Type::ConstantArray:
      result = cast<ConstantArrayType> (TP)->getSize().getSExtValue();
      break;
    case Type::Vector:
      result = cast<VectorType> (TP)->getNumElements();
      break;
    default:
      break;
    }
  }
  return result;
}

CXType lfort_getArrayElementType(CXType CT) {
  QualType ET = QualType();
  QualType T = GetQualType(CT);
  const Type *TP = T.getTypePtrOrNull();

  if (TP) {
    switch (TP->getTypeClass()) {
    case Type::ConstantArray:
      ET = cast<ConstantArrayType> (TP)->getElementType();
      break;
    default:
      break;
    }
  }
  return MakeCXType(ET, GetPgm(CT));
}

long long lfort_getArraySize(CXType CT) {
  long long result = -1;
  QualType T = GetQualType(CT);
  const Type *TP = T.getTypePtrOrNull();

  if (TP) {
    switch (TP->getTypeClass()) {
    case Type::ConstantArray:
      result = cast<ConstantArrayType> (TP)->getSize().getSExtValue();
      break;
    default:
      break;
    }
  }
  return result;
}

CXString lfort_getDeclObjCTypeEncoding(CXCursor C) {
  if (!lfort_isDeclaration(C.kind))
    return cxstring::createCXString("");

  Decl *D = static_cast<Decl*>(C.data[0]);
  CXProgram Pgm = static_cast<CXProgram>(C.data[2]);
  ASTUnit *AU = static_cast<ASTUnit*>(Pgm->PgmData);
  ASTContext &Ctx = AU->getASTContext();
  std::string encoding;

  if (ObjCMethodDecl *OMD = dyn_cast<ObjCMethodDecl>(D))  {
    if (Ctx.getObjCEncodingForMethodDecl(OMD, encoding))
      return cxstring::createCXString("?");
  } else if (ObjCPropertyDecl *OPD = dyn_cast<ObjCPropertyDecl>(D)) 
    Ctx.getObjCEncodingForPropertyDecl(OPD, NULL, encoding);
  else if (SubprogramDecl *FD = dyn_cast<SubprogramDecl>(D))
    Ctx.getObjCEncodingForSubprogramDecl(FD, encoding);
  else {
    QualType Ty;
    if (TypeDecl *TD = dyn_cast<TypeDecl>(D))
      Ty = Ctx.getTypeDeclType(TD);
    if (ValueDecl *VD = dyn_cast<ValueDecl>(D))
      Ty = VD->getType();
    else return cxstring::createCXString("?");
    Ctx.getObjCEncodingForType(Ty, encoding);
  }

  return cxstring::createCXString(encoding);
}

} // end: extern "C"
