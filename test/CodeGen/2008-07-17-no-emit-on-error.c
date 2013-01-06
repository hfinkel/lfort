// RUN: rm -f %t1.bc
// RUN: %lfort_cc1 -DPASS %s -emit-llvm-bc -o %t1.bc
// RUN: opt %t1.bc -disable-output
// RUN: rm -f %t1.bc
// RUN: not %lfort_cc1 %s -emit-llvm-bc -o %t1.bc
// RUN: not opt %t1.bc -disable-output

void f() {
}

#ifndef PASS
void g() {
  *10;
}
#endif
