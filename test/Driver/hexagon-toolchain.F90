! REQUIRES: hexagon-registered-target

! -----------------------------------------------------------------------------
! Test standard include paths
! -----------------------------------------------------------------------------

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK001 %s
! CHECK001: "-cc1" {{.*}} "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/lib/gcc/hexagon/4.4.0/include"
! CHECK001:   "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/lib/gcc/hexagon/4.4.0/include-fixed"
! CHECK001:   "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/hexagon/include"
! CHECK001-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"

! -----------------------------------------------------------------------------
! Test -nostdinc, -nostdlibinc
! -----------------------------------------------------------------------------

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -nostdinc \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK003 %s
! CHECK003: "-cc1"
! CHECK003-NOT: "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/lib/gcc/hexagon/4.4.0/include"
! CHECK003-NOT: "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/lib/gcc/hexagon/4.4.0/include-fixed"
! CHECK003-NOT: "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/hexagon/include"
! CHECK003-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -nostdlibinc \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK004 %s
! CHECK004: "-cc1"
! CHECK004-NOT: "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/lib/gcc/hexagon/4.4.0/include"
! CHECK004-NOT: "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/lib/gcc/hexagon/4.4.0/include-fixed"
! CHECK004-NOT: "-internal-externc-isystem" "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/hexagon/include"
! CHECK004-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"

! -----------------------------------------------------------------------------
! Test -march=<archname> -mcpu=<archname> -mv<number>
! -----------------------------------------------------------------------------
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -march=hexagonv3 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK007 %s
! CHECK007: "-cc1" {{.*}} "-target-cpu" "hexagonv3"
! CHECK007-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"{{.*}} "-march=v3"
! CHECK007-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-ld"{{.*}} "-mv3"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -mcpu=hexagonv5 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK008 %s
! CHECK008: "-cc1" {{.*}} "-target-cpu" "hexagonv5"
! CHECK008-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"{{.*}} "-march=v5"
! CHECK008-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-ld"{{.*}} "-mv5"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -mv2 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK009 %s
! CHECK009: "-cc1" {{.*}} "-target-cpu" "hexagonv2"
! CHECK009-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"{{.*}} "-march=v2"
! CHECK009-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-ld"{{.*}} "-mv2"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK010 %s
! CHECK010: "-cc1" {{.*}} "-target-cpu" "hexagonv4"
! CHECK010-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-as"{{.*}} "-march=v4"
! CHECK010-NEXT: "{{.*}}/Inputs/hexagon_tree/qc/bin/../../gnu/bin/hexagon-ld"{{.*}} "-mv4"

! -----------------------------------------------------------------------------
! Test Linker related args
! -----------------------------------------------------------------------------

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! Defaults for C
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK011 %s
! CHECK011: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK011-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK011-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK011-NOT: "-static"
! CHECK011-NOT: "-shared"
! CHECK011: "{{.*}}/hexagon/lib/v4/crt0_standalone.o"
! CHECK011: "{{.*}}/hexagon/lib/v4/crt0.o"
! CHECK011: "{{.*}}/hexagon/lib/v4/init.o"
! CHECK011: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK011: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK011: "-L{{.*}}/lib/gcc"
! CHECK011: "-L{{.*}}/hexagon/lib/v4"
! CHECK011: "-L{{.*}}/hexagon/lib"
! CHECK011: "{{[^"]+}}.o"
! CHECK011: "--start-group" "-lstandalone" "-lc" "-lgcc" "--end-group"
! CHECK011: "{{.*}}/hexagon/lib/v4/fini.o"

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! Additional Libraries (-L)
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -Lone -L two -L three \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK013 %s
! CHECK013: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK013-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK013-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK013: "{{.*}}/hexagon/lib/v4/crt0_standalone.o"
! CHECK013: "{{.*}}/hexagon/lib/v4/crt0.o"
! CHECK013: "{{.*}}/hexagon/lib/v4/init.o"
! CHECK013: "-Lone" "-Ltwo" "-Lthree"
! CHECK013: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK013: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK013: "-L{{.*}}/lib/gcc"
! CHECK013: "-L{{.*}}/hexagon/lib/v4"
! CHECK013: "-L{{.*}}/hexagon/lib"
! CHECK013: "{{[^"]+}}.o"
! CHECK013: "--start-group" "-lstandalone" "-lc" "-lgcc" "--end-group"
! CHECK013: "{{.*}}/hexagon/lib/v4/fini.o"

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! -static, -shared
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -static \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK014 %s
! CHECK014: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK014-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK014-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK014: "-static"
! CHECK014: "{{.*}}/hexagon/lib/v4/crt0_standalone.o"
! CHECK014: "{{.*}}/hexagon/lib/v4/crt0.o"
! CHECK014: "{{.*}}/hexagon/lib/v4/init.o"
! CHECK014: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK014: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK014: "-L{{.*}}/lib/gcc"
! CHECK014: "-L{{.*}}/hexagon/lib/v4"
! CHECK014: "-L{{.*}}/hexagon/lib"
! CHECK014: "{{[^"]+}}.o"
! CHECK014: "--start-group" "-lstandalone" "-lc" "-lgcc" "--end-group"
! CHECK014: "{{.*}}/hexagon/lib/v4/fini.o"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -shared \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK015 %s
! CHECK015: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK015-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK015-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK015: "-shared" "-call_shared"
! CHECK015-NOT: crt0_standalone.o
! CHECK015-NOT: crt0.o
! CHECK015: "{{.*}}/hexagon/lib/v4/G0/initS.o"
! CHECK015: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4/G0"
! CHECK015: "-L{{.*}}/lib/gcc/hexagon/4.4.0/G0"
! CHECK015: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK015: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK015: "-L{{.*}}/lib/gcc"
! CHECK015: "-L{{.*}}/hexagon/lib/v4/G0"
! CHECK015: "-L{{.*}}/hexagon/lib/G0"
! CHECK015: "-L{{.*}}/hexagon/lib/v4"
! CHECK015: "-L{{.*}}/hexagon/lib"
! CHECK015: "{{[^"]+}}.o"
! CHECK015: "--start-group"
! CHECK015-NOT: "-lstandalone"
! CHECK015-NOT: "-lc"
! CHECK015: "-lgcc"
! CHECK015: "--end-group"
! CHECK015: "{{.*}}/hexagon/lib/v4/G0/finiS.o"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -shared \
! RUN:   -static \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK016 %s
! CHECK016: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK016-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK016-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK016: "-shared" "-call_shared" "-static"
! CHECK016-NOT: crt0_standalone.o
! CHECK016-NOT: crt0.o
! CHECK016: "{{.*}}/hexagon/lib/v4/G0/init.o"
! CHECK016: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4/G0"
! CHECK016: "-L{{.*}}/lib/gcc/hexagon/4.4.0/G0"
! CHECK016: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK016: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK016: "-L{{.*}}/lib/gcc"
! CHECK016: "-L{{.*}}/hexagon/lib/v4/G0"
! CHECK016: "-L{{.*}}/hexagon/lib/G0"
! CHECK016: "-L{{.*}}/hexagon/lib/v4"
! CHECK016: "-L{{.*}}/hexagon/lib"
! CHECK016: "{{[^"]+}}.o"
! CHECK016: "--start-group"
! CHECK016-NOT: "-lstandalone"
! CHECK016-NOT: "-lc"
! CHECK016: "-lgcc"
! CHECK016: "--end-group"
! CHECK016: "{{.*}}/hexagon/lib/v4/G0/fini.o"

! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! -moslib
! - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -moslib=first -moslib=second \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK020 %s
! CHECK020: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK020-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK020-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK020-NOT: "-static"
! CHECK020-NOT: "-shared"
! CHECK020-NOT: crt0_standalone.o
! CHECK020: "{{.*}}/hexagon/lib/v4/crt0.o"
! CHECK020: "{{.*}}/hexagon/lib/v4/init.o"
! CHECK020: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK020: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK020: "-L{{.*}}/lib/gcc"
! CHECK020: "-L{{.*}}/hexagon/lib/v4"
! CHECK020: "-L{{.*}}/hexagon/lib"
! CHECK020: "{{[^"]+}}.o"
! CHECK020: "--start-group"
! CHECK020: "-lfirst" "-lsecond"
! CHECK020-NOT: "-lstandalone"
! CHECK020: "-lc" "-lgcc" "--end-group"
! CHECK020: "{{.*}}/hexagon/lib/v4/fini.o"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -moslib=first -moslib=second -moslib=standalone\
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK021 %s
! CHECK021: "{{.*}}lfort{{.*}}" "-cc1"
! CHECK021-NEXT: "{{.*}}/bin/hexagon-as"{{.*}}
! CHECK021-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK021-NOT: "-static"
! CHECK021-NOT: "-shared"
! CHECK021: "{{.*}}/hexagon/lib/v4/crt0_standalone.o"
! CHECK021: "{{.*}}/hexagon/lib/v4/crt0.o"
! CHECK021: "{{.*}}/hexagon/lib/v4/init.o"
! CHECK021: "-L{{.*}}/lib/gcc/hexagon/4.4.0/v4"
! CHECK021: "-L{{.*}}/lib/gcc/hexagon/4.4.0"
! CHECK021: "-L{{.*}}/lib/gcc"
! CHECK021: "-L{{.*}}/hexagon/lib/v4"
! CHECK021: "-L{{.*}}/hexagon/lib"
! CHECK021: "{{[^"]+}}.o"
! CHECK021: "--start-group"
! CHECK021: "-lfirst" "-lsecond"
! CHECK021: "-lstandalone"
! CHECK021: "-lc" "-lgcc" "--end-group"
! CHECK021: "{{.*}}/hexagon/lib/v4/fini.o"

! -----------------------------------------------------------------------------
! pic, small data threshold
! -----------------------------------------------------------------------------
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK023 %s
! CHECK023:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK023:        "-mrelocation-model" "static"
! CHECK023-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK023-NOT:    "-G{{[0-9]+}}"
! CHECK023-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK023-NOT:    "-G{{[0-9]+}}"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -fpic \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK024 %s
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -fPIC \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK024 %s
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -fPIC \
! RUN:   -msmall_data_threshold=8 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK024 %s
! CHECK024:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK024-NOT:    "-mrelocation-model" "static"
! CHECK024:        "-pic-level" "{{[12]}}"
! CHECK024:        "-mllvm" "-hexagon-small-data-threshold=0"
! CHECK024-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK024:        "-G0"
! CHECK024-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK024:        "-G0"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -G=8 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK025 %s
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -G 8 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK025 %s
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -msmall-data-threshold=8 \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK025 %s
! CHECK025:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK025:        "-mrelocation-model" "static"
! CHECK025:        "-mllvm" "-hexagon-small-data-threshold=8"
! CHECK025-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK025:        "-G8"
! CHECK025-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK025:        "-G8"

! -----------------------------------------------------------------------------
! pie
! -----------------------------------------------------------------------------
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -pie \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK026 %s
! CHECK026:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK026-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK026-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK026:        "-pie"

! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -pie -shared \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK027 %s
! CHECK027:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK027-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK027-NEXT: "{{.*}}/bin/hexagon-ld"
! CHECK027-NOT:    "-pie"

! -----------------------------------------------------------------------------
! Misc Defaults
! -----------------------------------------------------------------------------
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK028 %s
! CHECK028:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK028:        "-mqdsp6-compat"
! CHECK028:        "-Wreturn-type"
! CHECK028-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK028-NEXT: "{{.*}}/bin/hexagon-ld"

! -----------------------------------------------------------------------------
! Test Assembler related args
! -----------------------------------------------------------------------------
! RUN: %lfort -### -target hexagon-unknown-linux     \
! RUN:   -ccc-install-dir %S/Inputs/hexagon_tree/qc/bin \
! RUN:   -gdwarf-2 \
! RUN:   -Wa,--noexecstack,--trap \
! RUN:   -Xassembler --keep-locals \
! RUN:   %s 2>&1 \
! RUN:   | FileCheck -check-prefix=CHECK029 %s
! CHECK029:      "{{.*}}lfort{{.*}}" "-cc1"
! CHECK029-NEXT: "{{.*}}/bin/hexagon-as"
! CHECK029:        "-gdwarf-2" "--noexecstack" "--trap" "--keep-locals"
! CHECK029-NEXT: "{{.*}}/bin/hexagon-ld"
