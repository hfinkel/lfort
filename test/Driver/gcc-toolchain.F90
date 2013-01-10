! Test that gcc-toolchain option is working correctly
!
! RUN: %lfort -no-canonical-prefixes %s -### -o %t 2>&1 \
! RUN:     -target i386-unknown-linux \
! RUN:     -gcc-toolchain %S/Inputs/ubuntu_11.04_multiarch_tree/usr \
! RUN:   | FileCheck %s
!
! CHECK: "-internal-isystem" "/usr/local/include"
!
! Test for linker toolchain detection. Note that only the '-L' flags will use
! the same precise formatting of the path as the '-internal-system' flags
! above, so we just blanket wildcard match the 'crtbegin.o'.
! CHECK: "{{[^"]*}}ld{{(.exe)?}}"
! CHECK: "[[TOOLCHAIN:[^"]+]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5/crtbegin.o"
! CHECK: "-L[[TOOLCHAIN]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5"
! CHECK: "-L[[TOOLCHAIN]]/usr/lib/i386-linux-gnu/gcc/i686-linux-gnu/4.5/../../../.."
