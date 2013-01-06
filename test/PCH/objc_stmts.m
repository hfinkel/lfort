// Test this without pch.
// RUN: %lfort_cc1 -include %S/objc_stmts.h -emit-llvm -fobjc-exceptions -o - %s
// RUN: %lfort_cc1 -include %S/objc_stmts.h -ast-print -fobjc-exceptions -o - %s | FileCheck %s

// Test with pch.
// RUN: %lfort_cc1 -x objective-c -emit-pch -fobjc-exceptions -o %t %S/objc_stmts.h
// RUN: %lfort_cc1 -include-pch %t -emit-llvm -fobjc-exceptions -o - %s 
// RUN: %lfort_cc1 -include-pch %t -ast-print -fobjc-exceptions -o - %s | FileCheck %s

// CHECK: @catch(A *a)
// CHECK: @catch(B *b)
// CHECK: @catch()
