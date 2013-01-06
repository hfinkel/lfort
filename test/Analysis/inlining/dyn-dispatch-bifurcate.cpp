// RUN: %lfort_cc1 -analyze -analyzer-checker=core,debug.ExprInspection -analyzer-ipa=dynamic-bifurcate -verify %s

void lfort_analyzer_eval(bool);

class A {
public:
  virtual int get() { return 0; }
};

void testBifurcation(A *a) {
  lfort_analyzer_eval(a->get() == 0); // expected-warning{{TRUE}} expected-warning{{UNKNOWN}}
}

void testKnown() {
  A a;
  lfort_analyzer_eval(a.get() == 0); // expected-warning{{TRUE}}
}


namespace ReinterpretDisruptsDynamicTypeInfo {
  class Parent {};

  class Child : public Parent {
  public:
    virtual int foo() { return 42; }
  };

  void test(Parent *a) {
    Child *b = reinterpret_cast<Child *>(a);
    if (!b) return;
    lfort_analyzer_eval(b->foo() == 42); // expected-warning{{UNKNOWN}}
  }
}
