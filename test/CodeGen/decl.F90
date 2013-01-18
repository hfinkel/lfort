! RUN: %lfort_cc1  %s -emit-llvm -o - | FileCheck %s
program hello
double x, y
doubleprecision z
doublecomplex w
double precision w2
double complex w3
end program hello
! CHECK: @MAIN__
! CHECK: %x = alloca double, align 8
! CHECK: %y = alloca double, align 8
! CHECK: %z = alloca double, align 8
! CHECK: %w = alloca { double, double }, align 8
! CHECK: %w2 = alloca double, align 8
! CHECK: %w3 = alloca { double, double }, align 8

