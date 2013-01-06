// Test this without pch.
// RUN: %lfort_cc1 -include %s -emit-llvm -o - %s | FileCheck %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %s
// RUN: %lfort_cc1 -include-pch %t -emit-llvm -o - %s | FileCheck %s

#ifndef HEADER
#define HEADER

struct Bar
{
  // CHECK: align 512
  int buffer[123] __attribute__((__aligned__(512)));
};

#else

void foo() {
  struct Bar bar;
}

#endif
