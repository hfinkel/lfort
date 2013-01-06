// RUN: %lfort -### %s 2>&1 | FileCheck %s -check-prefix=NORMAL
// NORMAL-NOT: -force-align-stack
// NORMAL: -mstackrealign

// RUN: %lfort -### -mstackrealign %s 2>&1 | FileCheck %s -check-prefix=MREALIGN
// MREALIGN: -force-align-stack
// MREALIGN: -mstackrealign

// RUN: %lfort -### -mno-stackrealign %s 2>&1 | \
// RUN: FileCheck %s -check-prefix=MNOREALIGN
// MNOREALIGN-NOT: -force-align-stack
// MNOREALIGN-NOT: -mstackrealign
