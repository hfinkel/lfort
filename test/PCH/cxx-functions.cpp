// Test this without pch.
// RUN: %lfort_cc1 -include %S/cxx-functions.h -fsyntax-only -verify %s

// RUN: %lfort_cc1 -x c++-header -emit-pch -o %t %S/cxx-functions.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s

// expected-no-diagnostics


void test_foo() {
  foo();
}
