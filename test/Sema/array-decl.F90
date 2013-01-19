! RUN: %lfort_cc1 -triple x86_64-apple-darwin -fsyntax-only -Wunused-variable -verify %s
program hello
character(-7) c8 ! expected-error {{'c8' declared as an array with a negative size}}
end program hello

