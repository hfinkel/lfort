// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %S/pch__VA_ARGS__.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -Weverything %s 2>&1 | FileCheck %s

#define mylog(...) printf(__VA_ARGS__)
// CHECK-NOT: warning: __VA_ARGS__ can only appear in the expansion of a C99 variadic macro
