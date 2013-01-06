// RUN: cp %s %t
// RUN: %lfort_cc1 -x c++ %s -std=c++11 -fsyntax-only -verify
// RUN: not %lfort_cc1 -x c++ %t -std=c++11 -fixit
// RUN: %lfort_cc1 -x c++ %t -std=c++11 -fsyntax-only

constexpr int main() { } // expected-error{{'main' is not allowed to be declared constexpr}}
