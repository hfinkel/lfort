// RUN: %lfort_cc1 -triple i386-apple-darwin9 -fobjc-runtime=macosx-fragile-10.5 -emit-llvm %s -o %t

@interface Foo 
{
	double d1,d3,d4;
}
@end

Foo* foo()
{
  Foo *f;
  
  // Both of these crash lfort nicely
  ++f;
  --f;
 f--;
 f++;
 return f;
}
