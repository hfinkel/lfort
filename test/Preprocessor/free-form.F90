program hello
* RUN: %lfort_cc1 -E %s -fno-caret-diagnostics -o - 2>&1 | FileCheck -strict-whitespace %s
* CHECK: program hello
10 end &
* CHECK: 10 end&
program hello
* CHECK: program hello

