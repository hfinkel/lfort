// RUN: %lfort_cc1 -fsyntax-only -verify -pedantic %s

#pragma lfort diagnostic pop // expected-warning{{pragma diagnostic pop could not pop, no matching push}}

#pragma lfort diagnostic puhs // expected-warning {{pragma diagnostic expected 'error', 'warning', 'ignored', 'fatal', 'push', or 'pop'}}

int a = 'df'; // expected-warning{{multi-character character constant}}

#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wmultichar"

int b = 'df'; // no warning.
#pragma lfort diagnostic pop

int c = 'df';  // expected-warning{{multi-character character constant}}

#pragma lfort diagnostic pop // expected-warning{{pragma diagnostic pop could not pop, no matching push}}
