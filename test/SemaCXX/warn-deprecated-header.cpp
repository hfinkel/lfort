// RUN: %lfort_cc1 -fsyntax-only -fdeprecated-macro -verify %s
// RUN: %lfort_cc1 -fsyntax-only -Werror %s

// expected-warning@+2 {{This file is deprecated.}}
#ifdef __DEPRECATED
#warning This file is deprecated.
#endif
