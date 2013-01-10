C RUN: %lfort_cc1  %s -emit-llvm -o - | FileCheck %s
      program hello
* this is also a comment
C this is a comment
      enD
     @program hello
C FIXME: This should be @MAIN__
C CHECK: @main__
