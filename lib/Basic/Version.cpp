//===- Version.cpp - LFort Version Number -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines several version-related utility functions for LFort.
//
//===----------------------------------------------------------------------===//

#include "lfort/Basic/Version.h"
#include "lfort/Basic/LLVM.h"
#include "llvm/Config/config.h"
#include "llvm/Support/raw_ostream.h"
#include <cstdlib>
#include <cstring>

namespace lfort {

std::string getLFortRepositoryPath() {
#if defined(LFORT_REPOSITORY_STRING)
  return LFORT_REPOSITORY_STRING;
#else
#ifdef SVN_REPOSITORY
  StringRef URL(SVN_REPOSITORY);
#else
  StringRef URL("");
#endif

  // If the SVN_REPOSITORY is empty, try to use the SVN keyword. This helps us
  // pick up a tag in an SVN export, for example.
  static StringRef SVNRepository("$URL$");
  if (URL.empty()) {
    URL = SVNRepository.slice(SVNRepository.find(':'),
                              SVNRepository.find("/lib/Basic"));
  }

  // Strip off version from a build from an integration branch.
  URL = URL.slice(0, URL.find("/src/tools/lfort"));

  // Trim path prefix off, assuming path came from standard cfe path.
  size_t Start = URL.find("cfe/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start + 4);

  return URL;
#endif
}

std::string getLLVMRepositoryPath() {
#ifdef LLVM_REPOSITORY
  StringRef URL(LLVM_REPOSITORY);
#else
  StringRef URL("");
#endif

  // Trim path prefix off, assuming path came from standard llvm path.
  // Leave "llvm/" prefix to distinguish the following llvm revision from the
  // lfort revision.
  size_t Start = URL.find("llvm/");
  if (Start != StringRef::npos)
    URL = URL.substr(Start);

  return URL;
}

std::string getLFortRevision() {
#ifdef SVN_REVISION
  return SVN_REVISION;
#else
  return "";
#endif
}

std::string getLLVMRevision() {
#ifdef LLVM_REVISION
  return LLVM_REVISION;
#else
  return "";
#endif
}

std::string getLFortFullRepositoryVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
  std::string Path = getLFortRepositoryPath();
  std::string Revision = getLFortRevision();
  if (!Path.empty() || !Revision.empty()) {
    OS << '(';
    if (!Path.empty())
      OS << Path;
    if (!Revision.empty()) {
      if (!Path.empty())
        OS << ' ';
      OS << Revision;
    }
    OS << ')';
  }  
  // Support LLVM in a separate repository.
  std::string LLVMRev = getLLVMRevision();
  if (!LLVMRev.empty() && LLVMRev != Revision) {
    OS << " (";    
    std::string LLVMRepo = getLLVMRepositoryPath();
    if (!LLVMRepo.empty())
      OS << LLVMRepo << ' ';
    OS << LLVMRev << ')';
  }
  return OS.str();
}

std::string getLFortFullVersion() {
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef LFORT_VENDOR
  OS << LFORT_VENDOR;
#endif
  OS << "lfort version " LFORT_VERSION_STRING " "
     << getLFortFullRepositoryVersion();

  // If vendor supplied, include the base LLVM version as well.
#ifdef LFORT_VENDOR
  OS << " (based on LLVM " << PACKAGE_VERSION << ")";
#endif

  return OS.str();
}

std::string getLFortFullCPPVersion() {
  // The version string we report in __VERSION__ is just a compacted version of
  // the one we report on the command line.
  std::string buf;
  llvm::raw_string_ostream OS(buf);
#ifdef LFORT_VENDOR
  OS << LFORT_VENDOR;
#endif
  OS << "LFort " LFORT_VERSION_STRING " " << getLFortFullRepositoryVersion();
  return OS.str();
}

} // end namespace lfort
