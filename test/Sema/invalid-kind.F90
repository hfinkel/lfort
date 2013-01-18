! RUN: %lfort_cc1 -fsyntax-only -Wunused-variable -verify %s
program hello
logical(kind = 2+1) q ! expected-error {{kind for the type logical evaluates to 3; allowed values are: 1}}
end program hello

