// RUN: %lfort_cc1 -E %s | FileCheck %s
// CHECK: #pragma GCC diagnostic warning "-Wall"
#pragma GCC diagnostic warning "-Wall"
// CHECK: #pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wall"
// CHECK: #pragma GCC diagnostic error "-Wall"
#pragma GCC diagnostic error "-Wall"
// CHECK: #pragma GCC diagnostic fatal "-Wall"
#pragma GCC diagnostic fatal "-Wall"
// CHECK: #pragma GCC diagnostic push
#pragma GCC diagnostic push
// CHECK: #pragma GCC diagnostic pop
#pragma GCC diagnostic pop

// CHECK: #pragma lfort diagnostic warning "-Wall"
#pragma lfort diagnostic warning "-Wall"
// CHECK: #pragma lfort diagnostic ignored "-Wall"
#pragma lfort diagnostic ignored "-Wall"
// CHECK: #pragma lfort diagnostic error "-Wall"
#pragma lfort diagnostic error "-Wall"
// CHECK: #pragma lfort diagnostic fatal "-Wall"
#pragma lfort diagnostic fatal "-Wall"
// CHECK: #pragma lfort diagnostic push
#pragma lfort diagnostic push
// CHECK: #pragma lfort diagnostic pop
#pragma lfort diagnostic pop
