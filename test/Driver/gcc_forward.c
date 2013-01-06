// Check that we don't try to forward -Xlfort or -mlinker-version to GCC.
//
// RUN: %lfort -target powerpc-unknown-unknown \
// RUN:   -c %s \
// RUN:   -Xlfort foo-bar \
// RUN:   -mlinker-version=10 -### 2> %t
// RUN: FileCheck < %t %s
//
// CHECK: gcc{{.*}}"
// CHECK-NOT: "-mlinker-version=10"
// CHECK-NOT: "-Xlfort"
// CHECK-NOT: "foo-bar"
// CHECK: gcc_forward
