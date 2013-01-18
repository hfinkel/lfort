! RUN: %lfort_cc1 -triple x86_64-apple-darwin -fsyntax-only -Wunused-variable -verify %s
program hello
logical(kind = 2+1) q ! expected-error {{kind for the type logical evaluates to 3; allowed values are: 1}}
real(kind = 7) x ! expected-error {{kind for the type real evaluates to 7; allowed values are: 4, 8 and 16}}
end program hello

