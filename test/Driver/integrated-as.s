// RUN: %lfort -### -c -integrated-as %s 2>&1 | FileCheck %s

// REQUIRES: lfort-driver

// CHECK: cc1as
// CHECK-NOT: -relax-all
