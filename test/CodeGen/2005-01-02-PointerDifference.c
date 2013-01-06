// RUN: %lfort_cc1  %s -emit-llvm -o - | FileCheck %s

// CHECK: sdiv exact
int Diff(int *P, int *Q) { return P-Q; }
