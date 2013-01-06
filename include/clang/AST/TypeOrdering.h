//===-------------- TypeOrdering.h - Total ordering for types -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file provides a function objects and specializations that
//  allow QualType values to be sorted, used in std::maps, std::sets,
//  llvm::DenseMaps, and llvm::DenseSets.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_TYPE_ORDERING_H
#define LLVM_LFORT_TYPE_ORDERING_H

#include "lfort/AST/CanonicalType.h"
#include "lfort/AST/Type.h"
#include <functional>

namespace lfort {

/// QualTypeOrdering - Function object that provides a total ordering
/// on QualType values.
struct QualTypeOrdering : std::binary_function<QualType, QualType, bool> {
  bool operator()(QualType T1, QualType T2) const {
    return std::less<void*>()(T1.getAsOpaquePtr(), T2.getAsOpaquePtr());
  }
};

}

namespace llvm {
  template<class> struct DenseMapInfo;

  template<> struct DenseMapInfo<lfort::QualType> {
    static inline lfort::QualType getEmptyKey() { return lfort::QualType(); }

    static inline lfort::QualType getTombstoneKey() {
      using lfort::QualType;
      return QualType::getFromOpaquePtr(reinterpret_cast<lfort::Type *>(-1));
    }

    static unsigned getHashValue(lfort::QualType Val) {
      return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
            ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
    }

    static bool isEqual(lfort::QualType LHS, lfort::QualType RHS) {
      return LHS == RHS;
    }
  };

  template<> struct DenseMapInfo<lfort::CanQualType> {
    static inline lfort::CanQualType getEmptyKey() { 
      return lfort::CanQualType(); 
    }
    
    static inline lfort::CanQualType getTombstoneKey() {
      using lfort::CanQualType;
      return CanQualType::getFromOpaquePtr(reinterpret_cast<lfort::Type *>(-1));
    }
    
    static unsigned getHashValue(lfort::CanQualType Val) {
      return (unsigned)((uintptr_t)Val.getAsOpaquePtr()) ^
      ((unsigned)((uintptr_t)Val.getAsOpaquePtr() >> 9));
    }
    
    static bool isEqual(lfort::CanQualType LHS, lfort::CanQualType RHS) {
      return LHS == RHS;
    }
  };
}

#endif
