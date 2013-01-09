! RUN: touch %t.o
! RUN: not %lfort -DCRASH -o %t.o -MMD -MF %t.d %s
! RUN: test ! -f %t.o
! RUN: test ! -f %t.d

! RUN: touch %t.o
! RUN: not %lfort -DMISSING -o %t.o -MMD -MF %t.d %s
! RUN: test ! -f %t.o
! RUN: test ! -f %t.d

! RUN: touch %t.o
! RUN: not %lfort -o %t.o -MMD -MF %t.d %s
! RUN: test ! -f %t.o
! RUN: test -f %t.d

! REQUIRES: shell
! REQUIRES: crash-recovery

#ifdef CRASH
#pragma lfort __debug crash
#elif defined(MISSING)
#include "nonexistent.h"
#else
invalid C code
#endif
