// Test this without pch.
// RUN: %lfort_cc1 -include %S/objc_property.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -x objective-c -emit-pch -o %t %S/objc_property.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

void func() {
 TestProperties *xx = [TestProperties alloc];
 xx.value = 5;
}
