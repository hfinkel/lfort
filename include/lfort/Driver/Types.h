//===--- Types.h - Input & Temporary Driver Types ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_DRIVER_TYPES_H_
#define LFORT_DRIVER_TYPES_H_

#include "lfort/Driver/Phases.h"

namespace lfort {
namespace driver {
namespace types {
  enum ID {
    TY_INVALID,
#define TYPE(NAME, ID, PP_TYPE, TEMP_SUFFIX, FLAGS) TY_##ID,
#include "lfort/Driver/Types.def"
#undef TYPE
    TY_LAST
  };

  /// getTypeName - Return the name of the type for \p Id.
  const char *getTypeName(ID Id);

  /// getPreprocessedType - Get the ID of the type for this input when
  /// it has been preprocessed, or INVALID if this input is not
  /// preprocessed.
  ID getPreprocessedType(ID Id);

  /// getTypeTempSuffix - Return the suffix to use when creating a
  /// temp file of this type, or null if unspecified.
  const char *getTypeTempSuffix(ID Id);

  /// onlyAssembleType - Should this type only be assembled.
  bool onlyAssembleType(ID Id);

  /// onlyPrecompileType - Should this type only be precompiled.
  bool onlyPrecompileType(ID Id);

  /// canTypeBeUserSpecified - Can this type be specified on the
  /// command line (by the type name); this is used when forwarding
  /// commands to gcc.
  bool canTypeBeUserSpecified(ID Id);

  /// appendSuffixForType - When generating outputs of this type,
  /// should the suffix be appended (instead of replacing the existing
  /// suffix).
  bool appendSuffixForType(ID Id);

  /// canLipoType - Is this type acceptable as the output of a
  /// universal build (currently, just the Nothing, Image, and Object
  /// types).
  bool canLipoType(ID Id);

  /// isAcceptedByLFort - Can lfort handle this input type.
  bool isAcceptedByLFort(ID Id);

  /// isF77 - Is this a Fortran 77 input.
  bool isF77(ID Id);

  /// lookupTypeForExtension - Lookup the type to use for the file
  /// extension \p Ext.
  ID lookupTypeForExtension(const char *Ext);

  /// lookupTypeForTypSpecifier - Lookup the type to use for a user
  /// specified type name.
  ID lookupTypeForTypeSpecifier(const char *Name);

  /// getNumCompilationPhases - Return the complete number of phases
  /// to be done for this type.
  unsigned getNumCompilationPhases(ID Id);

  /// getCompilationPhase - Return the \p N th compilation phase to
  /// be done for this type.
  phases::ID getCompilationPhase(ID Id, unsigned N);
  
  /// lookupF77TypeForFortranType - Lookup Fortran 77 input type that corresponds to given
  /// Fortran type (used for lfort77 emulation of gfortran behaviour)
  ID lookupF77TypeForFortranType(ID Id);

} // end namespace types
} // end namespace driver
} // end namespace lfort

#endif
