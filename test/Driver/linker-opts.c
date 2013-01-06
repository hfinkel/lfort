// RUN: env LIBRARY_PATH=%T/test1 %lfort -x c %s -### 2>&1 | FileCheck %s
// CHECK: "-L{{.*}}/test1"

// GCC driver is used as linker on cygming. It should be aware of LIBRARY_PATH.
// XFAIL: win32
// REQUIRES: lfort-driver
