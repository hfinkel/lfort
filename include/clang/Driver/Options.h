//===--- Options.h - Option info & table ------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_DRIVER_OPTIONS_H
#define LFORT_DRIVER_OPTIONS_H

namespace lfort {
namespace driver {
  class OptTable;

namespace options {
  enum ID {
    OPT_INVALID = 0, // This is not an option ID.
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR) OPT_##ID,
#include "lfort/Driver/Options.inc"
    LastOption
#undef OPTION
#undef PREFIX
  };
}

  OptTable *createDriverOptTable();
}
}

#endif
