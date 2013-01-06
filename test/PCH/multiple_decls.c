// Test this without pch.
// RUN: %lfort_cc1 -include %S/multiple_decls.h -fsyntax-only -ast-print -o - %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %S/multiple_decls.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -ast-print -o - %s 

void f0(char c) {
  wide(c);
}

struct wide w;
struct narrow n;

void f1(int i) {
  narrow(i);
}
