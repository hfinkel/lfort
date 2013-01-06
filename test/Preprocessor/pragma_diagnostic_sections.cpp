// RUN: %lfort_cc1 -fsyntax-only -Wall -Wunused-macros -Wunused-parameter -Wno-uninitialized -verify %s

// rdar://8365684
struct S {
    void m1() { int b; while (b==b); } // expected-warning {{always evaluates to true}}

#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wtautological-compare"
    void m2() { int b; while (b==b); }
#pragma lfort diagnostic pop

    void m3() { int b; while (b==b); } // expected-warning {{always evaluates to true}}
};

//------------------------------------------------------------------------------

#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wtautological-compare"
template <typename T>
struct TS {
    void m() { T b; while (b==b); }
};
#pragma lfort diagnostic pop

void f() {
    TS<int> ts;
    ts.m();
}

//------------------------------------------------------------------------------

#define UNUSED_MACRO1 // expected-warning {{macro is not used}}

#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wunused-macros"
#define UNUSED_MACRO2
#pragma lfort diagnostic pop

//------------------------------------------------------------------------------

#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wreturn-type"
int g() { }
#pragma lfort diagnostic pop

//------------------------------------------------------------------------------

void ww(
#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wunused-parameter"
        int x,
#pragma lfort diagnostic pop
        int y) // expected-warning {{unused}}
{
}

//------------------------------------------------------------------------------

struct S2 {
    int x, y;
    S2() : 
#pragma lfort diagnostic push
#pragma lfort diagnostic ignored "-Wreorder"
    y(),
    x()
#pragma lfort diagnostic pop
    {}
};

//------------------------------------------------------------------------------

// rdar://8790245
#define MYMACRO \
    _Pragma("lfort diagnostic push") \
    _Pragma("lfort diagnostic ignored \"-Wunknown-pragmas\"") \
    _Pragma("lfort diagnostic pop")
MYMACRO
#undef MYMACRO

//------------------------------------------------------------------------------
