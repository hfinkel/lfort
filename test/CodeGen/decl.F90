! RUN: %lfort_cc1 -triple x86_64-apple-darwin %s -emit-llvm -o - | FileCheck %s
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
real(kind = 4) x2
real(kind = 8) x3
real(kind = 16) x4
! CHECK: %x2 = alloca float, align 4
! CHECK: %x3 = alloca double, align 8
! CHECK: %x4 = alloca x86_fp80, align 16
complex(4) r
complex(8) r2
complex(16) r3
! CHECK: %r = alloca { float, float }, align 4
! CHECK: %r2 = alloca { double, double }, align 8
! CHECK: %r3 = alloca { x86_fp80, x86_fp80 }, align 16
end program hello
