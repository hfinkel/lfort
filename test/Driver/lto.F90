! -emit-llvm, -flto, and -O4 all cause a switch to llvm-bc object files.
! RUN: %lfort -ccc-print-phases -c %s -flto 2> %t.log
! RUN: grep '2: compiler, {1}, lto-bc' %t.log
! RUN: %lfort -ccc-print-phases -c %s -O4 2> %t.log
! RUN: grep '2: compiler, {1}, lto-bc' %t.log

! and -emit-llvm doesn't alter pipeline (unfortunately?).
! RUN: %lfort -ccc-print-phases %s -emit-llvm 2> %t.log
! RUN: grep '0: input, ".*lto.F90", f90-cpp-input' %t.log
! RUN: grep '1: preprocessor, {0}, f90' %t.log
! RUN: grep '2: compiler, {1}, lto-bc' %t.log
! RUN: grep '3: linker, {2}, image' %t.log

! llvm-bc and llvm-ll outputs need to match regular suffixes
! (unfortunately).
! RUN: %lfort %s -emit-llvm -save-temps -### 2> %t.log
! RUN: grep '"-o" ".*lto\.f90" "-x" "f90-cpp-input" ".*lto\.F90"' %t.log
! RUN: grep '"-o" ".*lto\.o" .*".*lto\.f90"' %t.log
! RUN: grep '".*a.out" .*".*lto\.o"' %t.log

! RUN: %lfort %s -emit-llvm -S -### 2> %t.log
! RUN: grep '"-o" ".*lto\.s" "-x" "f90-cpp-input" ".*lto\.F90"' %t.log

