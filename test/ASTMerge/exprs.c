// RUN: %lfort_cc1 -emit-pch -o %t.1.ast %S/Inputs/exprs1.c
// RUN: %lfort_cc1 -emit-pch -o %t.2.ast %S/Inputs/exprs2.c
// RUN: %lfort_cc1 -ast-merge %t.1.ast -ast-merge %t.2.ast -fsyntax-only -verify %s
// expected-no-diagnostics

