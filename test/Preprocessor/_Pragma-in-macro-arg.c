// RUN: %lfort_cc1 %s -verify -Wconversion

// Don't crash (rdar://11168596)
#define A(desc) _Pragma("lfort diagnostic push")  _Pragma("lfort diagnostic ignored \"-Wparentheses\"") _Pragma("lfort diagnostic pop")
#define B(desc) A(desc)
B(_Pragma("lfort diagnostic ignored \"-Wparentheses\""))


#define EMPTY(x)
#define INACTIVE(x) EMPTY(x)

#define ID(x) x
#define ACTIVE(x) ID(x)

// This should be ignored..
INACTIVE(_Pragma("lfort diagnostic ignored \"-Wconversion\""))

#define IGNORE_CONV _Pragma("lfort diagnostic ignored \"-Wconversion\"") _Pragma("lfort diagnostic ignored \"-Wconversion\"")

// ..as should this.
INACTIVE(IGNORE_CONV)

#define IGNORE_POPPUSH(Pop, Push, W, D) Push W D Pop
IGNORE_POPPUSH(_Pragma("lfort diagnostic pop"), _Pragma("lfort diagnostic push"),
               _Pragma("lfort diagnostic ignored \"-Wconversion\""), int q = (double)1.0);

int x1 = (double)1.0; // expected-warning {{implicit conversion}}

ACTIVE(_Pragma) ("lfort diagnostic ignored \"-Wconversion\"")) // expected-error {{_Pragma takes a parenthesized string literal}} \
                                      expected-error {{expected identifier or '('}} expected-error {{expected ')'}} expected-note {{to match this '('}}

// This should disable the warning.
ACTIVE(IGNORE_CONV)

int x2 = (double)1.0;
