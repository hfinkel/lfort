// Test this without pch.
// RUN: %lfort_cc1 -include %S/enum.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %S/enum.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

int i = Red;

int return_enum_constant() {
  int result = aRoundShape;
  return result;
}

enum Shape s = Triangle;
