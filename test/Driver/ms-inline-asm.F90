! RUN: %lfort -target x86_64-apple-darwin10 \
! RUN:   -### -fsyntax-only -fasm-blocks %s 2> %t
! RUN: FileCheck --check-prefix=CHECK-BLOCKS < %t %s

! RUN: %lfort -target x86_64-apple-darwin10 \
! RUN:   -### -fsyntax-only -fno-asm-blocks -fasm-blocks %s 2> %t
! RUN: FileCheck --check-prefix=CHECK-BLOCKS < %t %s

! CHECK-BLOCKS: "-fasm-blocks"

! RUN: %lfort -target x86_64-apple-darwin10 \
! RUN:   -### -fsyntax-only -fasm-blocks -fno-asm-blocks %s 2> %t
! RUN: FileCheck --check-prefix=CHECK-NO-BLOCKS < %t %s

! CHECK-NO-BLOCKS-NOT: "-fasm-blocks"
