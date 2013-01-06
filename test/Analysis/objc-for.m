// RUN: %lfort_cc1 -analyze -analyzer-checker=osx.cocoa.Loops,debug.ExprInspection -verify %s

void lfort_analyzer_eval(int);

#define nil ((id)0)

@protocol NSFastEnumeration
- (int)countByEnumeratingWithState:(void *)state objects:(id *)objects count:(unsigned)count;
@end

@interface NSObject
+ (instancetype)testObject;
@end

@interface NSEnumerator <NSFastEnumeration>
@end

@interface NSArray : NSObject <NSFastEnumeration>
- (NSEnumerator *)objectEnumerator;
@end

@interface NSDictionary : NSObject <NSFastEnumeration>
@end

@interface NSMutableDictionary : NSDictionary
@end

@interface NSSet : NSObject <NSFastEnumeration>
@end

@interface NSPointerArray : NSObject <NSFastEnumeration>
@end

void test() {
  id x;
  for (x in [NSArray testObject])
    lfort_analyzer_eval(x != nil); // expected-warning{{TRUE}}

  for (x in [NSMutableDictionary testObject])
    lfort_analyzer_eval(x != nil); // expected-warning{{TRUE}}

  for (x in [NSSet testObject])
    lfort_analyzer_eval(x != nil); // expected-warning{{TRUE}}

  for (x in [[NSArray testObject] objectEnumerator])
    lfort_analyzer_eval(x != nil); // expected-warning{{TRUE}}

  for (x in [NSPointerArray testObject])
    lfort_analyzer_eval(x != nil); // expected-warning{{UNKNOWN}}
}

void testWithVarInFor() {
  for (id x in [NSArray testObject])
    lfort_analyzer_eval(x != nil); // expected-warning{{TRUE}}
  for (id x in [NSPointerArray testObject])
    lfort_analyzer_eval(x != nil); // expected-warning{{UNKNOWN}}
}

