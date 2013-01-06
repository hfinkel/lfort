// RUN: %lfort_cc1 -emit-llvm %s -o - -O1 | grep ashr
// RUN: %lfort_cc1 -emit-llvm %s -o - -O1 | not grep sdiv

long long test(int *A, int *B) {
  return A-B;
}
