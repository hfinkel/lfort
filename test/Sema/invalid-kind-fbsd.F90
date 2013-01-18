! RUN: %lfort_cc1 -triple powerpc64-unknown-freebsd-gnu -fsyntax-only -Wunused-variable -verify %s
program hello
! FreeBSD on PPC64 uses long double == double!
real(kind = 16) x ! expected-error {{kind for the type real evaluates to 16; allowed values are: 4 and 8}}
end program hello

