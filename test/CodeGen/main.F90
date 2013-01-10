! RUN: %lfort_cc1  %s -emit-llvm -o - | FileCheck %s
program hello
end program hello
! FIXME: This should be @MAIN__
! CHECK: @main__

