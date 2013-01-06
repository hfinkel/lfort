// RUN: %lfort_cc1 -E %s | grep -- '-"" , - "" , -"" , - ""'

#define A(b) -#b  ,  - #b  ,  -# b  ,  - # b
A()
