! RUN: %lfort_cc1  %s -emit-llvm -o - | FileCheck %s
program hello
end program hello
! CHECK: @MAIN__

