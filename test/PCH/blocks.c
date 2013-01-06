// Test this without pch.
// RUN: %lfort_cc1 -fblocks -include %S/blocks.h -fsyntax-only -emit-llvm -o - %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -fblocks -o %t %S/blocks.h
// RUN: %lfort_cc1 -fblocks -include-pch %t -fsyntax-only -emit-llvm -o - %s 

int do_add(int x, int y) { return add(x, y); }

int do_scaled_add(int a, int b, int s) {
  return scaled_add(a, b, s);
}
