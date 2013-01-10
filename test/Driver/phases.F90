! Basic compilation for various types of files.
! RUN: %lfort -target i386-unknown-unknown -ccc-print-phases -x f90-cpp-input %s -x f95-cpp-input %s -x f03-cpp-input %s -x f08-cpp-input -x f77-cpp-input %s -x none %s 2>&1 | FileCheck -check-prefix=BASIC %s

! BASIC: 0: input, "{{.*}}phases.F90", f90-cpp-input
! BASIC: 1: preprocessor, {0}, f90
! BASIC: 2: compiler, {1}, assembler
! BASIC: 3: assembler, {2}, object
! BASIC: 4: input, "{{.*}}phases.F90", f95-cpp-input
! BASIC: 5: preprocessor, {4}, f95
! BASIC: 6: compiler, {5}, assembler
! BASIC: 7: assembler, {6}, object
! BASIC: 8: input, "{{.*}}phases.F90", f03-cpp-input
! BASIC: 9: preprocessor, {8}, f03
! BASIC: 10: compiler, {9}, assembler
! BASIC: 11: assembler, {10}, object
! BASIC: 12: input, "{{.*}}phases.F90", f77-cpp-input
! BASIC: 13: preprocessor, {12}, f77
! BASIC: 14: compiler, {13}, assembler
! BASIC: 15: assembler, {14}, object
! BASIC: 16: input, "{{.*}}phases.F90", f90-cpp-input
! BASIC: 17: preprocessor, {16}, f90
! BASIC: 18: compiler, {17}, assembler
! BASIC: 19: assembler, {18}, object
! BASIC: 20: linker, {3, 7, 11, 15, 19}, image

! Universal linked image.
! RUN: %lfort -target i386-apple-darwin9 -ccc-print-phases -x f90-cpp-input %s -arch ppc -arch i386 2>&1 | FileCheck -check-prefix=ULI %s
! ULI: 0: input, "{{.*}}phases.F90", f90-cpp-input
! ULI: 1: preprocessor, {0}, f90
! ULI: 2: compiler, {1}, assembler
! ULI: 3: assembler, {2}, object
! ULI: 4: linker, {3}, image
! ULI: 5: bind-arch, "ppc", {4}, image
! ULI: 6: bind-arch, "i386", {4}, image
! ULI: 7: lipo, {5, 6}, image

! Universal object file.
! RUN: %lfort -target i386-apple-darwin9 -ccc-print-phases -c -x f90-cpp-input %s -arch ppc -arch i386 2>&1 | FileCheck -check-prefix=UOF %s
! UOF: 0: input, "{{.*}}phases.F90", f90-cpp-input
! UOF: 1: preprocessor, {0}, f90
! UOF: 2: compiler, {1}, assembler
! UOF: 3: assembler, {2}, object
! UOF: 4: bind-arch, "ppc", {3}, object
! UOF: 5: bind-arch, "i386", {3}, object
! UOF: 6: lipo, {4, 5}, object

! Analyzer
! RUN: %lfort -target i386-unknown-unknown -ccc-print-phases --analyze %s 2>&1 | FileCheck -check-prefix=ANALYZE %s
! ANALYZE: 0: input, "{{.*}}phases.F90", f90-cpp-input
! ANALYZE: 1: preprocessor, {0}, f90
! ANALYZE: 2: analyzer, {1}, plist

! FIXME: If we restore PCH support, then restore the PCH test.

