! RUN: %lfort     -target i386-unknown-unknown -fsanitize=thread %s -S -emit-llvm -o - | FileCheck %s
! RUN: %lfort -O1 -target i386-unknown-unknown -fsanitize=thread %s -S -emit-llvm -o - | FileCheck %s
! RUN: %lfort -O2 -target i386-unknown-unknown -fsanitize=thread %s -S -emit-llvm -o - | FileCheck %s
! RUN: %lfort -O3 -target i386-unknown-unknown -fsanitize=thread %s -S -emit-llvm -o - | FileCheck %s
! RUN: %lfort     -target i386-unknown-unknown -fsanitize=thread  %s -S -emit-llvm -o - | FileCheck %s
! Verify that -fsanitize=thread invokes tsan instrumentation.

end
! CHECK: __tsan_init
