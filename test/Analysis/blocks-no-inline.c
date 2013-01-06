// RUN: %lfort_cc1 -triple x86_64-apple-darwin10 -analyze -analyzer-checker=core,debug.ExprInspection -analyzer-ipa=none -fblocks -verify %s

void lfort_analyzer_eval(int);

void testInvalidation() {
  __block int i = 0;
  ^{
    ++i;
  }();

  // Under inlining, we will know that i == 1.
  lfort_analyzer_eval(i == 0); // expected-warning{{UNKNOWN}}
}
