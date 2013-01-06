// RUN: %lfort_cc1 -fsyntax-only %s
// RUN: %lfort_cc1 -ast-print %s
// RUN: %lfort_cc1 -ast-dump %s
// RUN: %lfort_cc1 -print-decl-contexts %s
// RUN: %lfort_cc1 -fdump-record-layouts %s

#include "cxx-language-features.inc"
