// Test this without pch.
// RUN: %lfort_cc1 -working-directory %S -I. -include working-directory.h %s -Wunused

// Test with pch.
// RUN: %lfort_cc1 -working-directory %S -x c++-header -emit-pch -o %t.pch -I. working-directory.h
// RUN: %lfort_cc1 -include-pch %t.pch -fsyntax-only %s -Wunused

void f() {
  // Instantiating A<char> will trigger a warning, which will end up trying to get the path to
  // the header that contains A.
  A<char> b;
}
