// Test this without pch.
// RUN: %lfort_cc1 -include %S/cxx-namespaces.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -x c++-header -emit-pch -o %t %S/cxx-namespaces.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

void m() {
  N::x = 0;
}
