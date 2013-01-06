// RUN: %lfort_cc1 %s -E | grep "^xy$"

#define A  x ## y
blah

A

