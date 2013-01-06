// RUN: %lfort     -target i386-unknown-unknown -faddress-sanitizer %s -S -emit-llvm -o - | FileCheck %s
// RUN: %lfort -O1 -target i386-unknown-unknown -faddress-sanitizer %s -S -emit-llvm -o - | FileCheck %s
// RUN: %lfort -O2 -target i386-unknown-unknown -faddress-sanitizer %s -S -emit-llvm -o - | FileCheck %s
// RUN: %lfort -O3 -target i386-unknown-unknown -faddress-sanitizer %s -S -emit-llvm -o - | FileCheck %s
// RUN: %lfort     -target i386-unknown-unknown -fsanitize=address  %s -S -emit-llvm -o - | FileCheck %s
// Verify that -faddress-sanitizer invokes asan instrumentation.

int foo(int *a) { return *a; }
// CHECK: __asan_init
