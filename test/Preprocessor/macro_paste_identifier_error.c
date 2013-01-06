// RUN: %lfort_cc1 -fms-extensions -Wno-invalid-token-paste %s -verify
// RUN: %lfort_cc1 -E -fms-extensions -Wno-invalid-token-paste %s | FileCheck %s
// RUN: %lfort_cc1 -E -fms-extensions -Wno-invalid-token-paste -x assembler-with-cpp %s | FileCheck %s
// expected-no-diagnostics

#define foo a ## b ## = 0
int foo;
// CHECK: int ab = 0;
