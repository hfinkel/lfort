! RUN: %lfort -fpack-struct -### %s 2> %t
! RUN: FileCheck < %t %s
! RUN: %lfort -fpack-struct=8 -### %s 2> %t
! RUN: FileCheck < %t %s --check-prefix=EQ

! CHECK: "-cc1"
! CHECK: "-fpack-struct=1"

! CHECK-EQ: "-cc1"
! CHECK-EQ: "-fpack-struct=8"
