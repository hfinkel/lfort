// Test this without pch.
// RUN: %lfort_cc1 -include %S/objc_methods.h -fsyntax-only -Wno-objc-root-class -verify %s

// Test with pch.
// RUN: %lfort_cc1 -x objective-c -emit-pch -o %t %S/objc_methods.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

void func() {
 TestPCH *xx;
 TestForwardClassDecl *yy;
// FIXME:
// AliasForTestPCH *zz;
 
 xx = [TestPCH alloc];
 [xx instMethod];
}
