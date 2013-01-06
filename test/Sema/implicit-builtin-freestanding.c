// RUN: %lfort_cc1 -fsyntax-only -verify -ffreestanding %s
// expected-no-diagnostics

int malloc(int a) { return a; }

