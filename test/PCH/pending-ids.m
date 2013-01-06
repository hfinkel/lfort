// Test for rdar://10278815

// Without PCH
// RUN: %lfort_cc1 -fsyntax-only -verify -include %s %s

// With PCH
// RUN: %lfort_cc1 %s -emit-pch -o %t
// RUN: %lfort_cc1 -emit-llvm-only -verify %s -include-pch %t -g

// expected-no-diagnostics

#ifndef HEADER
#define HEADER
//===----------------------------------------------------------------------===//
// Header

typedef char BOOL;

@interface NSString
+ (BOOL)meth;
@end

static NSString * const cake = @"cake";

//===----------------------------------------------------------------------===//
#else
//===----------------------------------------------------------------------===//

@interface Foo {
  BOOL ivar;
}
@end

//===----------------------------------------------------------------------===//
#endif
