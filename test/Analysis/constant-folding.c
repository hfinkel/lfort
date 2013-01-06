// RUN: %lfort_cc1 -analyze -analyzer-checker=core,debug.ExprInspection -verify %s

void lfort_analyzer_eval(int);

// There should be no warnings unless otherwise indicated.

void testComparisons (int a) {
  // Sema can already catch the simple comparison a==a,
  // since that's usually a logic error (and not path-dependent).
  int b = a;
  lfort_analyzer_eval(b == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b >= a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b <= a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b != a); // expected-warning{{FALSE}}
  lfort_analyzer_eval(b > a); // expected-warning{{FALSE}}
  lfort_analyzer_eval(b < a); // expected-warning{{FALSE}}
}

void testSelfOperations (int a) {
  lfort_analyzer_eval((a|a) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a&a) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a^a) == 0); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a-a) == 0); // expected-warning{{TRUE}}
}

void testIdempotent (int a) {
  lfort_analyzer_eval((a*1) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a/1) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a+0) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a-0) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a<<0) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a>>0) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a^0) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a&(~0)) == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a|0) == a); // expected-warning{{TRUE}}
}

void testReductionToConstant (int a) {
  lfort_analyzer_eval((a*0) == 0); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a&0) == 0); // expected-warning{{TRUE}}
  lfort_analyzer_eval((a|(~0)) == (~0)); // expected-warning{{TRUE}}
}

void testSymmetricIntSymOperations (int a) {
  lfort_analyzer_eval((2+a) == (a+2)); // expected-warning{{TRUE}}
  lfort_analyzer_eval((2*a) == (a*2)); // expected-warning{{TRUE}}
  lfort_analyzer_eval((2&a) == (a&2)); // expected-warning{{TRUE}}
  lfort_analyzer_eval((2^a) == (a^2)); // expected-warning{{TRUE}}
  lfort_analyzer_eval((2|a) == (a|2)); // expected-warning{{TRUE}}
}

void testAsymmetricIntSymOperations (int a) {
  lfort_analyzer_eval(((~0) >> a) == (~0)); // expected-warning{{TRUE}}
  lfort_analyzer_eval((0 >> a) == 0); // expected-warning{{TRUE}}
  lfort_analyzer_eval((0 << a) == 0); // expected-warning{{TRUE}}

  // Unsigned right shift shifts in zeroes.
  lfort_analyzer_eval(((~0U) >> a) != (~0U)); // expected-warning{{UNKNOWN}}
}

void testLocations (char *a) {
  char *b = a;
  lfort_analyzer_eval(b == a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b >= a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b <= a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b != a); // expected-warning{{FALSE}}
  lfort_analyzer_eval(b > a); // expected-warning{{FALSE}}
  lfort_analyzer_eval(b < a); // expected-warning{{FALSE}}
}

void testMixedTypeComparisons (char a, unsigned long b) {
  if (a != 0) return;
  if (b != 0x100) return;

  lfort_analyzer_eval(a <= b); // expected-warning{{TRUE}}
  lfort_analyzer_eval(b >= a); // expected-warning{{TRUE}}
  lfort_analyzer_eval(a != b); // expected-warning{{TRUE}}
}
