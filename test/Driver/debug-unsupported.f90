! RUN: %lfort -c -gstabs %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gstabs+ %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gcoff %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gxcoff %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gxcoff+ %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gvms %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gstabs1 %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gcoff2 %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gxcoff3 %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gvms0 %s 2>&1 | FileCheck %s
! RUN: %lfort -c -gtoggle %s 2>&1 | FileCheck %s
!
! CHECK: error: unsupported option
