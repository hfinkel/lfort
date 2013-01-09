//===--- LangOptions.h - C Language Family Language Options -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the lfort::LangOptions interface.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_LANGOPTIONS_H
#define LLVM_LFORT_LANGOPTIONS_H

#include "lfort/Basic/LLVM.h"
#include "lfort/Basic/ObjCRuntime.h"
#include "lfort/Basic/Visibility.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include <string>

namespace lfort {

/// Bitfields of LangOptions, split out from LangOptions in order to ensure that
/// this large collection of bitfields is a trivial class type.
class LangOptionsBase {
public:
  // Define simple language options (with no accessors).
#define LANGOPT(Name, Bits, Default, Description) unsigned Name : Bits;
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description)
#include "lfort/Basic/LangOptions.def"

protected:
  // Define language options of enumeration type. These are private, and will
  // have accessors (below).
#define LANGOPT(Name, Bits, Default, Description)
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  unsigned Name : Bits;
#include "lfort/Basic/LangOptions.def"
};

/// \brief Keeps track of the various options that can be
/// enabled, which controls the dialect of C or C++ that is accepted.
class LangOptions : public RefCountedBase<LangOptions>, public LangOptionsBase {
public:
  typedef lfort::Visibility Visibility;
  
  enum GCMode { NonGC, GCOnly, HybridGC };
  enum StackProtectorMode { SSPOff, SSPOn, SSPReq };
  
  enum SignedOverflowBehaviorTy {
    SOB_Undefined,  // Default C standard behavior.
    SOB_Defined,    // -fwrapv
    SOB_Trapping    // -ftrapv
  };

public:
  lfort::ObjCRuntime ObjCRuntime;

  std::string ObjCConstantStringClass;
  
  /// \brief The name of the handler function to be called when -ftrapv is
  /// specified.
  ///
  /// If none is specified, abort (GCC-compatible behaviour).
  std::string OverflowHandler;

  /// \brief The name of the current module.
  std::string CurrentModule;
  
  LangOptions();

  // Define accessors/mutators for language options of enumeration type.
#define LANGOPT(Name, Bits, Default, Description) 
#define ENUM_LANGOPT(Name, Type, Bits, Default, Description) \
  Type get##Name() const { return static_cast<Type>(Name); } \
  void set##Name(Type Value) { Name = static_cast<unsigned>(Value); }  
#include "lfort/Basic/LangOptions.def"
  
  bool isSignedOverflowDefined() const {
    return getSignedOverflowBehavior() == SOB_Defined;
  }

  /// \brief Reset all of the options that are not considered when building a
  /// module.
  void resetNonModularOptions();
};

/// \brief Floating point control options
class FPOptions {
public:
  unsigned fp_contract : 1;

  FPOptions() : fp_contract(0) {}

  FPOptions(const LangOptions &LangOpts) :
    fp_contract(LangOpts.DefaultFPContract) {}
};

/// \brief OpenCL volatile options
class OpenCLOptions {
public:
#define OPENCLEXT(nm)  unsigned nm : 1;
#include "lfort/Basic/OpenCLExtensions.def"

  OpenCLOptions() {
#define OPENCLEXT(nm)   nm = 0;
#include "lfort/Basic/OpenCLExtensions.def"
  }
};

/// \brief Describes the kind of translation unit being processed.
enum TranslationUnitKind {
  /// \brief The translation unit is a complete translation unit.
  TU_Complete,
  /// \brief The translation unit is a prefix to a translation unit, and is
  /// not complete.
  TU_Prefix,
  /// \brief The translation unit is a module.
  TU_Module
};
  
}  // end namespace lfort

#endif