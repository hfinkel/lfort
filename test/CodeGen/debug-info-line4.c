// RUN: %lfort %s -g -gcolumn-info -S -emit-llvm -o - | FileCheck %s
// Checks that lfort emits column information when -gcolumn-info is passed.

int foo(int a, int b) { int c = a + b;


  return c;
}

// Without column information we wouldn't change locations for b.
// CHECK: metadata !{i32 4, i32 20,
