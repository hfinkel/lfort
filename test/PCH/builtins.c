// Test this without pch.
// RUN: %lfort_cc1 -include %S/builtins.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %S/builtins.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

void hello() {
  printf("Hello, World!");
}
