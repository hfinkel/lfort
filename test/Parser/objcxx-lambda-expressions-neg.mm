// RUN: %lfort_cc1 -fsyntax-only -Wno-unused-value -verify %s

int main() {
  []{}; // expected-error {{expected expression}}
}
