// RUN: rm -rf %t
// RUN: %lfort -fmodule-cache-path %t -fmodules -x objective-c -I %S/Inputs -emit-ast -o %t.ast %s
// RUN: %lfort_cc1 -ast-print -x ast - < %t.ast | FileCheck %s

@import import_decl;
// CHECK: struct T

int main() {
  return 0;
}
