! RUN: %lfort_cc1 -E %s -fno-caret-diagnostics -o - 2>&1 | FileCheck %s
include 'fortran-include.inc'
program test
end program
! CHECK: included file
INCLUDE 'fortran-include.inc'
! CHECK: included file

