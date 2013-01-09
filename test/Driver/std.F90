! RUN: %lfort -std=c99 -trigraphs -std=gnu99 %s -E -o - | FileCheck -check-prefix=OVERRIDE %s
! OVERRIDE: ??(??)
! RUN: %lfort -ansi %s -E -o - | FileCheck -check-prefix=ANSI %s
! ANSI: []
! RUN: %lfort -std=gnu99 -trigraphs %s -E -o - | FileCheck -check-prefix=EXPLICIT %s
! EXPLICIT: []

??(??)
