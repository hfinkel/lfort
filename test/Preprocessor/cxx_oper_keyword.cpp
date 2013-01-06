// RUN: not %lfort_cc1 %s -E
// RUN: %lfort_cc1 %s -E -fno-operator-names

// Not valid in C++ unless -fno-operator-names is passed.
#define and foo


