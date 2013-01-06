// RUN: %lfort_cc1 %s -emit-llvm-only

struct A { virtual ~A(); };
struct B : A {
  ~B() { }
};
B x;

