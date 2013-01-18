! RUN: %lfort_cc1 -fsyntax-only -Wunused-variable -verify %s
program hello
double x ! expected-warning {{unused variable 'x'}}
end program hello

