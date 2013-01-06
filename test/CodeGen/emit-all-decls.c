// RUN: %lfort_cc1 -emit-llvm -o %t %s
// RUN: not grep "@foo" %t
// RUN: %lfort_cc1 -femit-all-decls -emit-llvm -o %t %s
// RUN: grep "@foo" %t

static void foo() {
  
}
