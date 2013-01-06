// Don't crash.

// RUN: cp %S/modified-header-crash.h %t.h
// RUN: %lfort_cc1 -DCAKE -x c-header %t.h -emit-pch -o %t
// RUN: echo 'int foobar;' >> %t.h
// RUN: not %lfort_cc1 %s -include-pch %t -fsyntax-only

// FIXME: It is intended to suppress this on win32.
// REQUIRES: ansi-escape-sequences

void f(void) {
  foo = 3;
}
