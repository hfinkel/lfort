// RUN: %lfort_cc1 -dM -undef -Dfoo=1 -E %s | FileCheck %s

// CHECK-NOT: #define __lfort__
// CHECK: #define foo 1
