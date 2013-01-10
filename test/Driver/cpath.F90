! RUN: mkdir -p %T/test1

! RUN: env "CPATH=%T/test1%{pathsep}%T/test2" %lfort -x c -E -v %s 2>&1 | FileCheck %s -check-prefix=CPATH
! CPATH: -I{{.*}}/test1
! CPATH: -I{{.*}}/test2
! CPATH: search starts here
! CPATH: test1
! CPATH: test2

