// Test this without pch.
// RUN: %lfort_cc1 -include %S/nonvisible-external-defs.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %S/nonvisible-external-defs.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

int g(int, float); // expected-error{{conflicting types}}

// expected-note{{previous declaration}}
