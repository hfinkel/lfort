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
logical*1 q4
! CHECK: %q = alloca i8, align 1
! CHECK: %q2 = alloca i8, align 1
! CHECK: %q3 = alloca i8, align 1
! CHECK: %q4 = alloca i8, align 1
real(kind = 4) x2
real(kind = 8) x3
real(kind = 16) x4
real*16 x5
! CHECK: %x2 = alloca float, align 4
! CHECK: %x3 = alloca double, align 8
! CHECK: %x4 = alloca x86_fp80, align 16
! CHECK: %x5 = alloca x86_fp80, align 16
complex(4) r
complex(8) r2
complex(16) r3
complex*32 r4
! CHECK: %r = alloca { float, float }, align 4
! CHECK: %r2 = alloca { double, double }, align 8
! CHECK: %r3 = alloca { x86_fp80, x86_fp80 }, align 16
! CHECK: %r4 = alloca { x86_fp80, x86_fp80 }, align 16
byte b
! CHECK: %b = alloca i8, align 1
integer i
integer*4 i2
integer(kind = 1) i3
integer(kind = 2) i4
integer(kind = 4) i5
integer(kind = 8) i6
! CHECK: %i = alloca i32, align 4
! CHECK: %i2 = alloca i32, align 4
! CHECK: %i3 = alloca i8, align 1
! CHECK: %i4 = alloca i16, align 2
! CHECK: %i5 = alloca i32, align 4
! CHECK: %i6 = alloca i64, align 8
character c
character(kind = 4) c2
! CHECK: %c = alloca [1 x i8], align 1
! CHECK: %c2 = alloca [1 x i32], align 4
character*3 c3
character*4, c4
character*(2+3) c5
character(kind = 1, len = 6) c6
character(kind = 4, len = 7) c7
character(len = 8) c8
! CHECK: %c3 = alloca [3 x i8], align 1
! CHECK: %c4 = alloca [4 x i8], align 1
! CHECK: %c5 = alloca [5 x i8], align 1
! CHECK: %c6 = alloca [6 x i8], align 1
! CHECK: %c7 = alloca [7 x i32], align 16
! CHECK: %c8 = alloca [8 x i8], align 1
end program hello
