// Test this without pch.
// RUN: %lfort_cc1 -include %S/Inputs/chain-ext_vector1.h -include %S/Inputs/chain-ext_vector2.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t1 %S/Inputs/chain-ext_vector1.h
// RUN: %lfort_cc1 -emit-pch -o %t2 %S/Inputs/chain-ext_vector2.h -include-pch %t1
// RUN: %lfort_cc1 -include-pch %t2 -fsyntax-only -verify %s

int test(float4 f4) {
  return f4.xy; // expected-error{{float2}}
}
