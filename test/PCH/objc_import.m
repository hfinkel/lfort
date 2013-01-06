// Test this without pch.
// RUN: %lfort_cc1 -include %S/objc_import.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -x objective-c -emit-pch -o %t %S/objc_import.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

#import "objc_import.h"

void func() {
 TestPCH *xx;

 xx = [TestPCH alloc];
 [xx instMethod];
}
