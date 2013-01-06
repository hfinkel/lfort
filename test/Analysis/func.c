// RUN: %lfort_cc1 -analyze -analyzer-checker=core,alpha.core,debug.ExprInspection -analyzer-store=region -verify %s

void lfort_analyzer_eval(int);

void f(void) {
  void (*p)(void);
  p = f;
  p = &f;
  p();
  (*p)();
}

void g(void (*fp)(void));

void f2() {
  g(f);
}

void f3(void (*f)(void), void (*g)(void)) {
  lfort_analyzer_eval(!f); // expected-warning{{UNKNOWN}}
  f();
  lfort_analyzer_eval(!f); // expected-warning{{FALSE}}

  lfort_analyzer_eval(!g); // expected-warning{{UNKNOWN}}
  (*g)();
  lfort_analyzer_eval(!g); // expected-warning{{FALSE}}
}
