// RUN: %lfort_cc1 -triple x86_64-apple-darwin10 -fsyntax-only -verify -fblocks %s
// RUN: %lfort_cc1 -fsyntax-only -verify -fblocks -fobjc-arc %s
// expected-no-diagnostics

struct X {
  __unsafe_unretained id object;
  int (^ __unsafe_unretained block)(int, int);
};

void f(struct X x) {
  x.object = 0;
  x.block = ^(int x, int y) { return x + y; };
}
