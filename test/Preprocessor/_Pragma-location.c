// RUN: %lfort_cc1 %s -fms-extensions -E | FileCheck %s
// We use -fms-extensions to test both _Pragma and __pragma.

// A long time ago the pragma lexer's buffer showed through in -E output.
// CHECK-NOT: scratch space

#define push_p _Pragma ("pack(push)")
push_p
// CHECK: #pragma pack(push)

push_p _Pragma("pack(push)") __pragma(pack(push))
// CHECK: #pragma pack(push)
// CHECK-NEXT: #line 11 "{{.*}}_Pragma-location.c"
// CHECK-NEXT: #pragma pack(push)
// CHECK-NEXT: #line 11 "{{.*}}_Pragma-location.c"
// CHECK-NEXT: #pragma pack(push)


#define __PRAGMA_PUSH_NO_EXTRA_ARG_WARNINGS _Pragma("lfort diagnostic push") \
_Pragma("lfort diagnostic ignored \"-Wformat-extra-args\"")
#define __PRAGMA_POP_NO_EXTRA_ARG_WARNINGS _Pragma("lfort diagnostic pop")

void test () {
  1;_Pragma("lfort diagnostic push") \
  _Pragma("lfort diagnostic ignored \"-Wformat-extra-args\"")
  _Pragma("lfort diagnostic pop")

  2;__PRAGMA_PUSH_NO_EXTRA_ARG_WARNINGS
  3;__PRAGMA_POP_NO_EXTRA_ARG_WARNINGS
}

// CHECK: void test () {
// CHECK-NEXT:   1;
// CHECK-NEXT: #line 24 "{{.*}}_Pragma-location.c"
// CHECK-NEXT: #pragma lfort diagnostic push
// CHECK-NEXT: #pragma lfort diagnostic ignored "-Wformat-extra-args"
// CHECK-NEXT: #pragma lfort diagnostic pop

// CHECK:   2;
// CHECK-NEXT: #line 28 "{{.*}}_Pragma-location.c"
// CHECK-NEXT: #pragma lfort diagnostic push
// CHECK-NEXT: #line 28 "{{.*}}_Pragma-location.c"
// CHECK-NEXT: #pragma lfort diagnostic ignored "-Wformat-extra-args"
// CHECK-NEXT:   3;
// CHECK-NEXT: #line 29 "{{.*}}_Pragma-location.c"
// CHECK-NEXT: #pragma lfort diagnostic pop
// CHECK-NEXT: }
