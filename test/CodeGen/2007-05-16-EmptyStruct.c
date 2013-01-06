// PR 1417
// RUN: %lfort_cc1   %s -emit-llvm -o - | FileCheck %s

// CHECK: global %struct.anon* null
struct { } *X;
