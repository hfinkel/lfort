//===- Version.h - LFort Version Number -------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines version macros and version-related utility functions
/// for LFort.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_BASIC_VERSION_H
#define LLVM_LFORT_BASIC_VERSION_H

#include "lfort/Basic/Version.inc"
#include "llvm/ADT/StringRef.h"

/// \brief Helper macro for LFORT_VERSION_STRING.
#define LFORT_MAKE_VERSION_STRING2(X) #X

#ifdef LFORT_VERSION_PATCHLEVEL
/// \brief Helper macro for LFORT_VERSION_STRING.
#define LFORT_MAKE_VERSION_STRING(X,Y,Z) LFORT_MAKE_VERSION_STRING2(X.Y.Z)

/// \brief A string that describes the LFort version number, e.g., "1.0".
#define LFORT_VERSION_STRING \
  LFORT_MAKE_VERSION_STRING(LFORT_VERSION_MAJOR,LFORT_VERSION_MINOR, \
                            LFORT_VERSION_PATCHLEVEL)
#else
/// \brief Helper macro for LFORT_VERSION_STRING.
#define LFORT_MAKE_VERSION_STRING(X,Y) LFORT_MAKE_VERSION_STRING2(X.Y)

/// \brief A string that describes the LFort version number, e.g., "1.0".
#define LFORT_VERSION_STRING \
  LFORT_MAKE_VERSION_STRING(LFORT_VERSION_MAJOR,LFORT_VERSION_MINOR)
#endif

namespace lfort {
  /// \brief Retrieves the repository path (e.g., Subversion path) that
  /// identifies the particular LFort branch, tag, or trunk from which this
  /// LFort was built.
  std::string getLFortRepositoryPath();

  /// \brief Retrieves the repository path from which LLVM was built.
  ///
  /// This supports LLVM residing in a separate repository from lfort.
  std::string getLLVMRepositoryPath();

  /// \brief Retrieves the repository revision number (or identifer) from which
  /// this LFort was built.
  std::string getLFortRevision();

  /// \brief Retrieves the repository revision number (or identifer) from which
  /// LLVM was built.
  ///
  /// If LFort and LLVM are in the same repository, this returns the same
  /// string as getLFortRevision.
  std::string getLLVMRevision();

  /// \brief Retrieves the full repository version that is an amalgamation of
  /// the information in getLFortRepositoryPath() and getLFortRevision().
  std::string getLFortFullRepositoryVersion();

  /// \brief Retrieves a string representing the complete lfort version,
  /// which includes the lfort version number, the repository version,
  /// and the vendor tag.
  std::string getLFortFullVersion();

  /// \brief Retrieves a string representing the complete lfort version suitable
  /// for use in the CPP __VERSION__ macro, which includes the lfort version
  /// number, the repository version, and the vendor tag.
  std::string getLFortFullCPPVersion();
}

#endif // LLVM_LFORT_BASIC_VERSION_H
