// RUN: %lfort_cc1 -emit-llvm %s  -o /dev/null


int foo(int *A, unsigned X) {
  return A[X];
}
