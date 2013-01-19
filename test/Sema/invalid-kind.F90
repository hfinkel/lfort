! RUN: %lfort_cc1 -triple x86_64-apple-darwin -fsyntax-only -Wunused-variable -verify %s
program hello
logical(kind = 2+1) q ! expected-error {{kind for the type logical evaluates to 3; allowed values are: 1}}
real(kind = 7) x ! expected-error {{kind for the type real evaluates to 7; allowed values are: 4, 8 and 16}}
complex(kind = 32) r ! expected-error {{kind for the type complex evaluates to 32; allowed values are: 4, 8 and 16}}
complex*4 r2 ! expected-error {{invalid old-style kind specifier for the type complex; allowed values are: 8, 16 and 32}}
integer(16) i ! expected-error {{kind for the type integer evaluates to 16; allowed values are: 1, 2, 4 and 8}}
character(kind = 2) c2 ! expected-error {{kind for the type character evaluates to 2; allowed values are: 1 and 4}}
end program hello

