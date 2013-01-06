// RUN: %lfort -### -pedantic -no-pedantic %s 2>&1 | FileCheck -check-prefix=NO_PEDANTIC %s
// RUN: %lfort -### -pedantic -Wno-pedantic %s 2>&1 | FileCheck -check-prefix=PEDANTIC %s
// NO_PEDANTIC-NOT: -pedantic
// RUN: %lfort -### -pedantic -pedantic -no-pedantic -pedantic %s 2>&1 | FileCheck -check-prefix=PEDANTIC %s
// RUN: %lfort -### -pedantic -pedantic -no-pedantic -Wpedantic %s 2>&1 | FileCheck -check-prefix=NO_PEDANTIC %s
// PEDANTIC: -pedantic
// REQUIRES: lfort-driver
