// RUN: %lfort_cc1 -triple armv7-unknown-unknown %s  -o /dev/null
// RUN: %lfort_cc1 -triple x86_64-unknown-unknown %s  -o /dev/null

int foo() {
#ifdef __arm__
  register int X __asm__("r1");
#else
  register int X __asm__("ebx");
#endif
  return X;
}
