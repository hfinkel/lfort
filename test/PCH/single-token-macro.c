// rdar://10588825

// Test this without pch.
// RUN: %lfort_cc1 %s -include %s -verify -fsyntax-only

// Test with pch.
// RUN: %lfort_cc1 %s -emit-pch -o %t
// RUN: %lfort_cc1 %s -include-pch %t -verify -fsyntax-only

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

#define __stdcall
#define STDCALL __stdcall

void STDCALL Foo(void);

#else

void STDCALL Foo(void)
{
}

#endif
