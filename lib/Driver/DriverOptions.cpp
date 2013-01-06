//===--- DriverOptions.cpp - Driver Options Table -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Driver/Options.h"
#include "lfort/Driver/OptTable.h"
#include "lfort/Driver/Option.h"

using namespace lfort::driver;
using namespace lfort::driver::options;

#define PREFIX(NAME, VALUE) const char *const NAME[] = VALUE;
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)
#include "lfort/Driver/Options.inc"
#undef OPTION
#undef PREFIX

static const OptTable::Info InfoTable[] = {
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)   \
  { PREFIX, NAME, HELPTEXT, METAVAR, OPT_##ID, Option::KIND##Class, PARAM, \
    FLAGS, OPT_##GROUP, OPT_##ALIAS },
#include "lfort/Driver/Options.inc"
};

namespace {

class DriverOptTable : public OptTable {
public:
  DriverOptTable()
    : OptTable(InfoTable, sizeof(InfoTable) / sizeof(InfoTable[0])) {}
};

}

OptTable *lfort::driver::createDriverOptTable() {
  return new DriverOptTable();
}
