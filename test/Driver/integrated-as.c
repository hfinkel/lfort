// RUN: %lfort -### -c -save-temps -integrated-as %s 2>&1 | FileCheck %s

// gcc is invoked instead of lfort-cc1as with gcc-driver -save-temps.
// REQUIRES: lfort-driver

// CHECK: cc1as
// CHECK: -relax-all
