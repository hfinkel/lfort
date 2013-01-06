// RUN: %lfort_cc1 -fsyntax-only  %s

void f(...) {
  int g(int(...));
}
