// Test this without pch.
// RUN: %lfort_cc1 -include %S/Inputs/chain-decls1.h -include %S/Inputs/chain-decls2.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t1 %S/Inputs/chain-decls1.h
// RUN: %lfort_cc1 -emit-pch -o %t2 %S/Inputs/chain-decls2.h -include-pch %t1
// RUN: %lfort_cc1 -include-pch %t2 -fsyntax-only -verify %s
// RUN: %lfort_cc1 -ast-print -include-pch %t2 %s | FileCheck %s

// expected-no-diagnostics

// CHECK: void f();
// CHECK: void g();

int h() {
  f();
  g();

  struct one x;
  one();
  struct two y;
  two();
  struct three z;

  many(0);
  struct many m;

  noret();
}
