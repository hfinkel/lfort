//===--- CC1AsOptions.cpp - LFort Assembler Options Table -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Driver/CC1AsOptions.h"
#include "lfort/Driver/OptTable.h"
#include "lfort/Driver/Option.h"
using namespace lfort;
using namespace lfort::driver;
using namespace lfort::driver::options;
using namespace lfort::driver::cc1asoptions;

#define PREFIX(NAME, VALUE) const char *const NAME[] = VALUE;
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)
#include "lfort/Driver/CC1AsOptions.inc"
#undef OPTION
#undef PREFIX

static const OptTable::Info CC1AsInfoTable[] = {
#define PREFIX(NAME, VALUE)
#define OPTION(PREFIX, NAME, ID, KIND, GROUP, ALIAS, FLAGS, PARAM, \
               HELPTEXT, METAVAR)   \
  { PREFIX, NAME, HELPTEXT, METAVAR, OPT_##ID, Option::KIND##Class, PARAM, \
    FLAGS, OPT_##GROUP, OPT_##ALIAS },
#include "lfort/Driver/CC1AsOptions.inc"
};

namespace {

class CC1AsOptTable : public OptTable {
public:
  CC1AsOptTable()
    : OptTable(CC1AsInfoTable,
               sizeof(CC1AsInfoTable) / sizeof(CC1AsInfoTable[0])) {}
};

}

OptTable *lfort::driver::createCC1AsOptTable() {
  return new CC1AsOptTable();
}
