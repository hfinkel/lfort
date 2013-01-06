// RUN: %lfort -emit-llvm -S -o %t1.ll -x c - < %s
// RUN: %lfort -emit-ast -o %t.ast %s
// RUN: %lfort -emit-llvm -S -o %t2.ll -x ast - < %t.ast
// RUN: diff %t1.ll %t2.ll

int main() {
  return 0;
}
