// RUN: %lfort_cc1 -emit-llvm %s -o %t

void *test(int i)
{
  return (void *)i;
}
