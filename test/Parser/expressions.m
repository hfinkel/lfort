// RUN: %lfort_cc1 -fsyntax-only -verify %s

void test1() {
  @"s";            // expected-warning {{expression result unused}}
}

