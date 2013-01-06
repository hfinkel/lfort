// Test this without pch.
// RUN: %lfort_cc1 %s -include %s -verify -fsyntax-only

// Test with pch.
// RUN: %lfort_cc1 %s -emit-pch -o %t
// RUN: %lfort_cc1 %s -include-pch %t -verify -fsyntax-only

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

#pragma lfort diagnostic ignored "-Wtautological-compare"

#else

void f() {
  int a = 0;
  int b = a==a;
}

#endif
