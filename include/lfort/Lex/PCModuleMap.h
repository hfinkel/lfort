//===--- PCModuleMap.h - Describe the layout of modules -----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the PCModuleMap interface, which describes the layout of a
// module as it relates to headers.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_LFORT_LEX_MODULEMAP_H
#define LLVM_LFORT_LEX_MODULEMAP_H

#include "lfort/Basic/LangOptions.h"
#include "lfort/Basic/PCModule.h"
#include "lfort/Basic/SourceManager.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/ADT/StringRef.h"
#include <string>

namespace lfort {
  
class DirectoryEntry;
class FileEntry;
class FileManager;
class DiagnosticConsumer;
class DiagnosticsEngine;
class PCModuleMapParser;
  
class PCModuleMap {
  SourceManager *SourceMgr;
  IntrusiveRefCntPtr<DiagnosticsEngine> Diags;
  const LangOptions &LangOpts;
  const TargetInfo *Target;
  
  /// \brief The directory used for LFort-supplied, builtin include headers,
  /// such as "stdint.h".
  const DirectoryEntry *BuiltinIncludeDir;
  
  /// \brief Language options used to parse the module map itself.
  ///
  /// These are always simple C language options.
  LangOptions MMapLangOpts;

  /// \brief The top-level modules that are known.
  llvm::StringMap<PCModule *> PCModules;

  /// \brief A header that is known to reside within a given module,
  /// whether it was included or excluded.
  class KnownHeader {
    llvm::PointerIntPair<PCModule *, 1, bool> Storage;

  public:
    KnownHeader() : Storage(0, false) { }
    KnownHeader(PCModule *M, bool Excluded) : Storage(M, Excluded) { }

    /// \brief Retrieve the module the header is stored in.
    PCModule *getPCModule() const { return Storage.getPointer(); }

    /// \brief Whether this header is explicitly excluded from the module.
    bool isExcluded() const { return Storage.getInt(); }

    /// \brief Whether this header is available in the module.
    bool isAvailable() const { 
      return !isExcluded() && getPCModule()->isAvailable(); 
    }

    // \brief Whether this known header is valid (i.e., it has an
    // associated module).
    operator bool() const { return Storage.getPointer() != 0; }
  };

  typedef llvm::DenseMap<const FileEntry *, KnownHeader> HeadersMap;

  /// \brief Mapping from each header to the module that owns the contents of the
  /// that header.
  HeadersMap Headers;
  
  /// \brief Mapping from directories with umbrella headers to the module
  /// that is generated from the umbrella header.
  ///
  /// This mapping is used to map headers that haven't explicitly been named
  /// in the module map over to the module that includes them via its umbrella
  /// header.
  llvm::DenseMap<const DirectoryEntry *, PCModule *> UmbrellaDirs;

  /// \brief A directory for which framework modules can be inferred.
  struct InferredDirectory {
    InferredDirectory() : InferPCModules(), InferSystemPCModules() { }

    /// \brief Whether to infer modules from this directory.
    unsigned InferPCModules : 1;

    /// \brief Whether the modules we infer are [system] modules.
    unsigned InferSystemPCModules : 1;

    /// \brief The names of modules that cannot be inferred within this
    /// directory.
    llvm::SmallVector<std::string, 2> ExcludedPCModules;
  };

  /// \brief A mapping from directories to information about inferring
  /// framework modules from within those directories.
  llvm::DenseMap<const DirectoryEntry *, InferredDirectory> InferredDirectories;

  friend class PCModuleMapParser;
  
  /// \brief Resolve the given export declaration into an actual export
  /// declaration.
  ///
  /// \param Mod The module in which we're resolving the export declaration.
  ///
  /// \param Unresolved The export declaration to resolve.
  ///
  /// \param Complain Whether this routine should complain about unresolvable
  /// exports.
  ///
  /// \returns The resolved export declaration, which will have a NULL pointer
  /// if the export could not be resolved.
  PCModule::ExportDecl 
  resolveExport(PCModule *Mod, const PCModule::UnresolvedExportDecl &Unresolved,
                bool Complain);
  
public:
  /// \brief Construct a new module map.
  ///
  /// \param FileMgr The file manager used to find module files and headers.
  /// This file manager should be shared with the header-search mechanism, since
  /// they will refer to the same headers.
  ///
  /// \param DC A diagnostic consumer that will be cloned for use in generating
  /// diagnostics.
  ///
  /// \param LangOpts Language options for this translation unit.
  ///
  /// \param Target The target for this translation unit.
  PCModuleMap(FileManager &FileMgr, const DiagnosticConsumer &DC,
            const LangOptions &LangOpts, const TargetInfo *Target);

  /// \brief Destroy the module map.
  ///
  ~PCModuleMap();

  /// \brief Set the target information.
  void setTarget(const TargetInfo &Target);

  /// \brief Set the directory that contains LFort-supplied include
  /// files, such as our stdarg.h or tgmath.h.
  void setBuiltinIncludeDir(const DirectoryEntry *Dir) {
    BuiltinIncludeDir = Dir;
  }

  /// \brief Retrieve the module that owns the given header file, if any.
  ///
  /// \param File The header file that is likely to be included.
  ///
  /// \returns The module that owns the given header file, or null to indicate
  /// that no module owns this header file.
  PCModule *findPCModuleForHeader(const FileEntry *File);

  /// \brief Determine whether the given header is part of a module
  /// marked 'unavailable'.
  bool isHeaderInUnavailablePCModule(const FileEntry *Header);

  /// \brief Retrieve a module with the given name.
  ///
  /// \param Name The name of the module to look up.
  ///
  /// \returns The named module, if known; otherwise, returns null.
  PCModule *findPCModule(StringRef Name);

  /// \brief Retrieve a module with the given name using lexical name lookup,
  /// starting at the given context.
  ///
  /// \param Name The name of the module to look up.
  ///
  /// \param Context The module context, from which we will perform lexical
  /// name lookup.
  ///
  /// \returns The named module, if known; otherwise, returns null.
  PCModule *lookupPCModuleUnqualified(StringRef Name, PCModule *Context);

  /// \brief Retrieve a module with the given name within the given context,
  /// using direct (qualified) name lookup.
  ///
  /// \param Name The name of the module to look up.
  /// 
  /// \param Context The module for which we will look for a submodule. If
  /// null, we will look for a top-level module.
  ///
  /// \returns The named submodule, if known; otherwose, returns null.
  PCModule *lookupPCModuleQualified(StringRef Name, PCModule *Context);
  
  /// \brief Find a new module or submodule, or create it if it does not already
  /// exist.
  ///
  /// \param Name The name of the module to find or create.
  ///
  /// \param Parent The module that will act as the parent of this submodule,
  /// or NULL to indicate that this is a top-level module.
  ///
  /// \param IsFramework Whether this is a framework module.
  ///
  /// \param IsExplicit Whether this is an explicit submodule.
  ///
  /// \returns The found or newly-created module, along with a boolean value
  /// that will be true if the module is newly-created.
  std::pair<PCModule *, bool> findOrCreatePCModule(StringRef Name, PCModule *Parent, 
                                               bool IsFramework,
                                               bool IsExplicit);

  /// \brief Determine whether we can infer a framework module a framework
  /// with the given name in the given
  ///
  /// \param ParentDir The directory that is the parent of the framework
  /// directory.
  ///
  /// \param Name The name of the module.
  ///
  /// \param IsSystem Will be set to 'true' if the inferred module must be a
  /// system module.
  ///
  /// \returns true if we are allowed to infer a framework module, and false
  /// otherwise.
  bool canInferFrameworkPCModule(const DirectoryEntry *ParentDir,
                               StringRef Name, bool &IsSystem);

  /// \brief Infer the contents of a framework module map from the given
  /// framework directory.
  PCModule *inferFrameworkPCModule(StringRef PCModuleName, 
                               const DirectoryEntry *FrameworkDir,
                               bool IsSystem, PCModule *Parent);
  
  /// \brief Retrieve the module map file containing the definition of the given
  /// module.
  ///
  /// \param PCModule The module whose module map file will be returned, if known.
  ///
  /// \returns The file entry for the module map file containing the given
  /// module, or NULL if the module definition was inferred.
  const FileEntry *getContainingPCModuleMapFile(PCModule *PCModule);

  /// \brief Resolve all of the unresolved exports in the given module.
  ///
  /// \param Mod The module whose exports should be resolved.
  ///
  /// \param Complain Whether to emit diagnostics for failures.
  ///
  /// \returns true if any errors were encountered while resolving exports,
  /// false otherwise.
  bool resolveExports(PCModule *Mod, bool Complain);

  /// \brief Infers the (sub)module based on the given source location and 
  /// source manager.
  ///
  /// \param Loc The location within the source that we are querying, along
  /// with its source manager.
  ///
  /// \returns The module that owns this source location, or null if no
  /// module owns this source location.
  PCModule *inferPCModuleFromLocation(FullSourceLoc Loc);
  
  /// \brief Sets the umbrella header of the given module to the given
  /// header.
  void setUmbrellaHeader(PCModule *Mod, const FileEntry *UmbrellaHeader);

  /// \brief Sets the umbrella directory of the given module to the given
  /// directory.
  void setUmbrellaDir(PCModule *Mod, const DirectoryEntry *UmbrellaDir);

  /// \brief Adds this header to the given module.
  /// \param Excluded Whether this header is explicitly excluded from the
  /// module; otherwise, it's included in the module.
  void addHeader(PCModule *Mod, const FileEntry *Header, bool Excluded);

  /// \brief Parse the given module map file, and record any modules we 
  /// encounter.
  ///
  /// \param File The file to be parsed.
  ///
  /// \returns true if an error occurred, false otherwise.
  bool parsePCModuleMapFile(const FileEntry *File);
    
  /// \brief Dump the contents of the module map, for debugging purposes.
  void dump();
  
  typedef llvm::StringMap<PCModule *>::const_iterator module_iterator;
  module_iterator module_begin() const { return PCModules.begin(); }
  module_iterator module_end()   const { return PCModules.end(); }
};
  
}
#endif
