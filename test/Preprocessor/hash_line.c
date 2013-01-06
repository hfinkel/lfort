// The 1 and # should not go on the same line.
// RUN: %lfort_cc1 %s -E | not grep "1 #"
// RUN: %lfort_cc1 %s -E | grep '^1$'
// RUN: %lfort_cc1 %s -E | grep '^      #$'
1
#define EMPTY
EMPTY #

