//===--- CC1AsOptions.h - LFort Assembler Options Table ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_DRIVER_CC1ASOPTIONS_H
#define LFORT_DRIVER_CC1ASOPTIONS_H

namespace lfort {
namespace driver {
  class OptTable;

namespace cc1asoptions {
  enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR) OPT_##ID,
#include "lfort/Driver/CC1AsOptions.inc"
    LastOption
#undef OPTION
#undef PREFIX
  };
}

  OptTable *createCC1AsOptTable();
}
}

#endif
