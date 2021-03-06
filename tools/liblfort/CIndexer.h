//===- CIndexer.h - LFort-C Source Indexing Library -------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines CIndexer, a subclass of Indexer that provides extra
// functionality needed by the CIndex library.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_CINDEXER_H
#define LLVM_LFORT_CINDEXER_H

#include "lfort-c/Index.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/Support/Path.h"
#include <vector>

namespace llvm {
  class CrashRecoveryContext;
}

namespace lfort {
  class ASTUnit;

class CIndexer {
  bool OnlyLocalDecls;
  bool DisplayDiagnostics;
  unsigned Options; // CXGlobalOptFlags.

  llvm::sys::Path ResourcesPath;

public:
 CIndexer() : OnlyLocalDecls(false), DisplayDiagnostics(false),
              Options(CXGlobalOpt_None) { }
  
  /// \brief Whether we only want to see "local" declarations (that did not
  /// come from a previous precompiled header). If false, we want to see all
  /// declarations.
  bool getOnlyLocalDecls() const { return OnlyLocalDecls; }
  void setOnlyLocalDecls(bool Local = true) { OnlyLocalDecls = Local; }
  
  bool getDisplayDiagnostics() const { return DisplayDiagnostics; }
  void setDisplayDiagnostics(bool Display = true) {
    DisplayDiagnostics = Display;
  }

  unsigned getCXGlobalOptFlags() const { return Options; }
  void setCXGlobalOptFlags(unsigned options) { Options = options; }

  bool isOptEnabled(CXGlobalOptFlags opt) const {
    return Options & opt;
  }

  /// \brief Get the path of the lfort resource files.
  std::string getLFortResourcesPath();
};

  /**
   * \brief Given a set of "unsaved" files, create temporary files and 
   * construct the lfort -cc1 argument list needed to perform the remapping.
   *
   * \returns true if an error occurred.
   */
  bool RemapFiles(unsigned num_unsaved_files,
                  struct CXUnsavedFile *unsaved_files,
                  std::vector<std::string> &RemapArgs,
                  std::vector<llvm::sys::Path> &TemporaryFiles);

  /// \brief Return the current size to request for "safety".
  unsigned GetSafetyThreadStackSize();

  /// \brief Set the current size to request for "safety" (or 0, if safety
  /// threads should not be used).
  void SetSafetyThreadStackSize(unsigned Value);

  /// \brief Execution the given code "safely", using crash recovery or safety
  /// threads when possible.
  ///
  /// \return False if a crash was detected.
  bool RunSafely(llvm::CrashRecoveryContext &CRC,
                 void (*Fn)(void*), void *UserData, unsigned Size = 0);

  /// \brief Set the thread priority to background.
  /// FIXME: Move to llvm/Support.
  void setThreadBackgroundPriority();

  /// \brief Print liblfort's resource usage to standard error.
  void PrintLiblfortResourceUsage(CXProgram Pgm);

  namespace cxindex {
    void printDiagsToStderr(ASTUnit *Unit);
  }
}

#endif
