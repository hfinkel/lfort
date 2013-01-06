//===--- BaseSubobject.h - BaseSubobject class ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides a definition of the BaseSubobject class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_AST_BASESUBOBJECT_H
#define LLVM_LFORT_AST_BASESUBOBJECT_H

#include "lfort/AST/CharUnits.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/type_traits.h"

namespace lfort {
  class CXXRecordDecl;

// BaseSubobject - Uniquely identifies a direct or indirect base class. 
// Stores both the base class decl and the offset from the most derived class to
// the base class. Used for vtable and VTT generation.
class BaseSubobject {
  /// Base - The base class declaration.
  const CXXRecordDecl *Base;
  
  /// BaseOffset - The offset from the most derived class to the base class.
  CharUnits BaseOffset;
  
public:
  BaseSubobject() { }
  BaseSubobject(const CXXRecordDecl *Base, CharUnits BaseOffset)
    : Base(Base), BaseOffset(BaseOffset) { }
  
  /// getBase - Returns the base class declaration.
  const CXXRecordDecl *getBase() const { return Base; }

  /// getBaseOffset - Returns the base class offset.
  CharUnits getBaseOffset() const { return BaseOffset; }

  friend bool operator==(const BaseSubobject &LHS, const BaseSubobject &RHS) {
    return LHS.Base == RHS.Base && LHS.BaseOffset == RHS.BaseOffset;
 }
};

} // end namespace lfort

namespace llvm {

template<> struct DenseMapInfo<lfort::BaseSubobject> {
  static lfort::BaseSubobject getEmptyKey() {
    return lfort::BaseSubobject(
      DenseMapInfo<const lfort::CXXRecordDecl *>::getEmptyKey(),
      lfort::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getEmptyKey()));
  }

  static lfort::BaseSubobject getTombstoneKey() {
    return lfort::BaseSubobject(
      DenseMapInfo<const lfort::CXXRecordDecl *>::getTombstoneKey(),
      lfort::CharUnits::fromQuantity(DenseMapInfo<int64_t>::getTombstoneKey()));
  }

  static unsigned getHashValue(const lfort::BaseSubobject &Base) {
    typedef std::pair<const lfort::CXXRecordDecl *, lfort::CharUnits> PairTy;
    return DenseMapInfo<PairTy>::getHashValue(PairTy(Base.getBase(),
                                                     Base.getBaseOffset()));
  }

  static bool isEqual(const lfort::BaseSubobject &LHS, 
                      const lfort::BaseSubobject &RHS) {
    return LHS == RHS;
  }
};

// It's OK to treat BaseSubobject as a POD type.
template <> struct isPodLike<lfort::BaseSubobject> {
  static const bool value = true;
};

}

#endif
