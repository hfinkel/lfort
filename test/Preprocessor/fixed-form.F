      program hello
C RUN: %lfort_cc1 -E %s -fno-caret-diagnostics -o - 2>&1 | FileCheck -strict-whitespace %s
c CHECK: {{      }}program hello
      end
     @program hello
C CHECK: {{     }}&program hello

