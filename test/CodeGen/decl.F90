! RUN: %lfort_cc1  %s -emit-llvm -o - | FileCheck %s
program hello
! CHECK: @MAIN__
double x, y
doubleprecision z
doublecomplex w
double precision w2
double complex w3
! CHECK: %x = alloca double, align 8
! CHECK: %y = alloca double, align 8
! CHECK: %z = alloca double, align 8
! CHECK: %w = alloca { double, double }, align 8
! CHECK: %w2 = alloca double, align 8
! CHECK: %w3 = alloca { double, double }, align 8
logical q
logical(1) q2
logical(kind = 1) q3
! CHECK: %q = alloca i8, align 1
! CHECK: %q2 = alloca i8, align 1
! CHECK: %q3 = alloca i8, align 1
end program hello
