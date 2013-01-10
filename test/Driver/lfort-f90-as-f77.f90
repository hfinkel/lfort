! RUN: %lfort77 -### %s 2>&1 | FileCheck %s
!
! PR5803
!
! CHECK: warning: treating 'f90' input as 'f77' when not in Fortran 77 mode, this behavior is deprecated
! CHECK: "-cc1" {{.*}} "-x" "f77"
