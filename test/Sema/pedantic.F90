! RUN: %lfort_cc1 -fsyntax-only -pedantic -verify %s
program hello
double complex x ! expected-warning {{the double complex intrinsic type is a non-standard extension}}
byte y ! expected-warning {{the byte intrinsic type is a non-standard extension}}
real*4 z ! expected-warning {{old-style kind specifiers are a non-standard extension}}
end program hello

