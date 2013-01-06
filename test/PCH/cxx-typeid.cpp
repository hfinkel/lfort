// Test this without pch.
// RUN: %lfort -include %S/cxx-typeid.h -fsyntax-only -Xlfort -verify %s

// RUN: %lfort -ccc-pch-is-pch -x c++-header -o %t.gch %S/cxx-typeid.h
// RUN: %lfort -ccc-pch-is-pch -include %t -fsyntax-only -Xlfort -verify %s 

// expected-no-diagnostics

void f() {
    (void)typeid(int);
}
