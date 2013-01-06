// Test this without pch.
// RUN: %lfort_cc1 -include %S/objc_container.h -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -x objective-c -emit-pch -o %t %S/objc_container.h
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 
// RUN: %lfort_cc1 -include-pch %t -ast-print %s | FileCheck -check-prefix=PRINT %s
// RUN: %lfort_cc1 -include-pch %t -emit-llvm -o - %s | FileCheck -check-prefix=IR %s

// expected-no-diagnostics

// CHECK-PRINT: id oldObject = array[10];
// CHECK-PRINT: array[10] = oldObject;
// CHECK-PRINT: oldObject = dictionary[key];
// CHECK-PRINT: dictionary[key] = newObject;

// CHECK-IR: define void @all() nounwind 
// CHECK-IR: {{call.*objc_msgSend}}
// CHECK-IR: {{call.*objc_msgSend}}
// CHECK-IR: {{call.*objc_msgSend}}
// CHECK-IR: {{call.*objc_msgSend}}
// CHECK-IR: ret void
