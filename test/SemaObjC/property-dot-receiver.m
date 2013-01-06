// RUN: %lfort_cc1 -fsyntax-only -verify -Wno-objc-root-class %s 
// RUN: %lfort_cc1 -x objective-c++ -fsyntax-only -verify -Wno-objc-root-class %s 
// expected-no-diagnostics
// rdar://8962253

@interface Singleton {
}
+ (Singleton*) instance;
@end

@implementation Singleton

- (void) someSelector { }

+ (Singleton*) instance { return 0; }

+ (void) compileError
{
     [Singleton.instance  someSelector]; // lfort issues error here
}

@end

