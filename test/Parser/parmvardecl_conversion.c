// RUN: %lfort_cc1 -fsyntax-only -verify %s
// expected-no-diagnostics

void f (int p[]) { p++; }

