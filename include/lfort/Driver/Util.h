//===--- Util.h - Common Driver Utilities -----------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_DRIVER_UTIL_H_
#define LFORT_DRIVER_UTIL_H_

#include "lfort/Basic/LLVM.h"

namespace lfort {
namespace driver {
  class Action;

  /// ArgStringList - Type used for constructing argv lists for subprocesses.
  typedef SmallVector<const char*, 16> ArgStringList;

  /// ActionList - Type used for lists of actions.
  typedef SmallVector<Action*, 3> ActionList;

} // end namespace driver
} // end namespace lfort

#endif
