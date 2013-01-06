// RUN: %lfort_cc1 -E %s | grep '^a: x$'
// RUN: %lfort_cc1 -E %s | grep '^b: x y, z,h$'
// RUN: %lfort_cc1 -E %s | grep '^c: foo(x)$'

#define A(b, c...) b c
a: A(x)
b: A(x, y, z,h)

#define B(b, c...) foo(b, ## c)
c: B(x)
