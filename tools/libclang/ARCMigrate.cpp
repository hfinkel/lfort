//===- ARCMigrate.cpp - LFort-C ARC Migration Library ---------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the main API hooks in the LFort-C ARC Migration library.
//
//===----------------------------------------------------------------------===//

#include "lfort-c/Index.h"
#include "CXString.h"
#include "lfort/ARCMigrate/ARCMT.h"
#include "lfort/Frontend/TextDiagnosticBuffer.h"
#include "llvm/Support/FileSystem.h"

using namespace lfort;
using namespace arcmt;

namespace {

struct Remap {
  std::vector<std::pair<std::string, std::string> > Vec;
};

} // anonymous namespace.

//===----------------------------------------------------------------------===//
// libLFort public APIs.
//===----------------------------------------------------------------------===//

extern "C" {

CXRemapping lfort_getRemappings(const char *migrate_dir_path) {
  bool Logging = ::getenv("LIBLFORT_LOGGING");

  if (!migrate_dir_path) {
    if (Logging)
      llvm::errs() << "lfort_getRemappings was called with NULL parameter\n";
    return 0;
  }

  bool exists = false;
  llvm::sys::fs::exists(migrate_dir_path, exists);
  if (!exists) {
    if (Logging) {
      llvm::errs() << "Error by lfort_getRemappings(\"" << migrate_dir_path
                   << "\")\n";
      llvm::errs() << "\"" << migrate_dir_path << "\" does not exist\n";
    }
    return 0;
  }

  TextDiagnosticBuffer diagBuffer;
  OwningPtr<Remap> remap(new Remap());

  bool err = arcmt::getFileRemappings(remap->Vec, migrate_dir_path,&diagBuffer);

  if (err) {
    if (Logging) {
      llvm::errs() << "Error by lfort_getRemappings(\"" << migrate_dir_path
                   << "\")\n";
      for (TextDiagnosticBuffer::const_iterator
             I = diagBuffer.err_begin(), E = diagBuffer.err_end(); I != E; ++I)
        llvm::errs() << I->second << '\n';
    }
    return 0;
  }

  return remap.take();
}

CXRemapping lfort_getRemappingsFromFileList(const char **filePaths,
                                            unsigned numFiles) {
  bool Logging = ::getenv("LIBLFORT_LOGGING");

  OwningPtr<Remap> remap(new Remap());

  if (numFiles == 0) {
    if (Logging)
      llvm::errs() << "lfort_getRemappingsFromFileList was called with "
                      "numFiles=0\n";
    return remap.take();
  }

  if (!filePaths) {
    if (Logging)
      llvm::errs() << "lfort_getRemappingsFromFileList was called with "
                      "NULL filePaths\n";
    return 0;
  }

  TextDiagnosticBuffer diagBuffer;
  SmallVector<StringRef, 32> Files;
  for (unsigned i = 0; i != numFiles; ++i)
    Files.push_back(filePaths[i]);

  bool err = arcmt::getFileRemappingsFromFileList(remap->Vec, Files,
                                                  &diagBuffer);

  if (err) {
    if (Logging) {
      llvm::errs() << "Error by lfort_getRemappingsFromFileList\n";
      for (TextDiagnosticBuffer::const_iterator
             I = diagBuffer.err_begin(), E = diagBuffer.err_end(); I != E; ++I)
        llvm::errs() << I->second << '\n';
    }
    return remap.take();
  }

  return remap.take();
}

unsigned lfort_remap_getNumFiles(CXRemapping map) {
  return static_cast<Remap *>(map)->Vec.size();
  
}

void lfort_remap_getFilenames(CXRemapping map, unsigned index,
                              CXString *original, CXString *transformed) {
  if (original)
    *original = cxstring::createCXString(
                                    static_cast<Remap *>(map)->Vec[index].first,
                                        /*DupString =*/ true);
  if (transformed)
    *transformed = cxstring::createCXString(
                                   static_cast<Remap *>(map)->Vec[index].second,
                                  /*DupString =*/ true);
}

void lfort_remap_dispose(CXRemapping map) {
  delete static_cast<Remap *>(map);
}

} // end: extern "C"
