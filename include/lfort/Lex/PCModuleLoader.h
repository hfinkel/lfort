//===--- PCModuleLoader.h - PCModule Loader Interface ---------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the PCModuleLoader interface, which is responsible for 
//  loading named modules.
//
//===----------------------------------------------------------------------===//
#ifndef LLVM_LFORT_LEX_MODULE_LOADER_H
#define LLVM_LFORT_LEX_MODULE_LOADER_H

#include "lfort/Basic/PCModule.h"
#include "lfort/Basic/SourceLocation.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ADT/PointerIntPair.h"

namespace lfort {

class IdentifierInfo;
class PCModule;

/// \brief A sequence of identifier/location pairs used to describe a particular
/// module or submodule, e.g., std.vector.
typedef llvm::ArrayRef<std::pair<IdentifierInfo*, SourceLocation> > 
  PCModuleIdPath;

/// \brief Describes the result of attempting to load a module.
class PCModuleLoadResult {
  llvm::PointerIntPair<PCModule *, 1, bool> Storage;

public:
  PCModuleLoadResult() : Storage() { }

  PCModuleLoadResult(PCModule *module, bool missingExpected)
    : Storage(module, missingExpected) { }

  operator PCModule *() const { return Storage.getPointer(); }

  /// \brief Determines whether the module, which failed to load, was
  /// actually a submodule that we expected to see (based on implying the
  /// submodule from header structure), but didn't materialize in the actual
  /// module.
  bool isMissingExpected() const { return Storage.getInt(); }
};

/// \brief Abstract interface for a module loader.
///
/// This abstract interface describes a module loader, which is responsible
/// for resolving a module name (e.g., "std") to an actual module file, and
/// then loading that module.
class PCModuleLoader {
public:
  virtual ~PCModuleLoader();
  
  /// \brief Attempt to load the given module.
  ///
  /// This routine attempts to load the module described by the given 
  /// parameters.
  ///
  /// \param ImportLoc The location of the 'import' keyword.
  ///
  /// \param Path The identifiers (and their locations) of the module
  /// "path", e.g., "std.vector" would be split into "std" and "vector".
  /// 
  /// \param Visibility The visibility provided for the names in the loaded
  /// module.
  ///
  /// \param IsInclusionDirective Indicates that this module is being loaded
  /// implicitly, due to the presence of an inclusion directive. Otherwise,
  /// it is being loaded due to an import declaration.
  ///
  /// \returns If successful, returns the loaded module. Otherwise, returns 
  /// NULL to indicate that the module could not be loaded.
  virtual PCModuleLoadResult loadPCModule(SourceLocation ImportLoc,
                                      PCModuleIdPath Path,
                                      PCModule::NameVisibilityKind Visibility,
                                      bool IsInclusionDirective) = 0;
};
  
}

#endif
