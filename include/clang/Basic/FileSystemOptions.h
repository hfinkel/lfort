//===--- FileSystemOptions.h - File System Options --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the lfort::FileSystemOptions interface.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_BASIC_FILESYSTEMOPTIONS_H
#define LLVM_LFORT_BASIC_FILESYSTEMOPTIONS_H

#include <string>

namespace lfort {

/// \brief Keeps track of options that affect how file operations are performed.
class FileSystemOptions {
public:
  /// \brief If set, paths are resolved as if the working directory was
  /// set to the value of WorkingDir.
  std::string WorkingDir;
};

} // end namespace lfort

#endif
