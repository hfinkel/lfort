! Basic binding.
! RUN: %lfort -target i386-unknown-unknown -ccc-print-bindings %s 2>&1 | FileCheck %s --check-prefix=CHECK01
! CHECK01: "lfort", inputs: ["{{.*}}bindings.F90"], output: "{{.*}}.s"
! CHECK01: "gcc::Assemble", inputs: ["{{.*}}.s"], output: "{{.*}}.o"
! CHECK01: "gcc::Link", inputs: ["{{.*}}.o"], output: "a.out"

! LFort control options

! RUN: %lfort -target i386-unknown-unknown -ccc-print-bindings -fsyntax-only %s 2>&1 | FileCheck %s --check-prefix=CHECK05
! CHECK05: "lfort", inputs: ["{{.*}}bindings.F90"], output: (nothing)

! RUN: %lfort -target i386-unknown-unknown -ccc-print-bindings -fsyntax-only -x f08-cpp-input %s 2>&1 | FileCheck %s --check-prefix=CHECK08
! CHECK08: "lfort", inputs: ["{{.*}}bindings.F90"], output: (nothing)

! RUN: %lfort -target i386-apple-darwin9 -ccc-print-bindings %s -S -arch ppc 2>&1 | FileCheck %s --check-prefix=CHECK11
! CHECK11: "lfort", inputs: ["{{.*}}bindings.F90"], output: "bindings.s"

! RUN: %lfort -target powerpc-unknown-unknown -ccc-print-bindings %s -S 2>&1 | FileCheck %s --check-prefix=CHECK12
! CHECK12: "lfort", inputs: ["{{.*}}bindings.F90"], output: "bindings.s"

! Darwin bindings
! RUN: %lfort -target i386-apple-darwin9 -no-integrated-as -ccc-print-bindings %s 2>&1 | FileCheck %s --check-prefix=CHECK14
! CHECK14: "lfort", inputs: ["{{.*}}bindings.F90"], output: "{{.*}}.s"
! CHECK14: "darwin::Assemble", inputs: ["{{.*}}.s"], output: "{{.*}}.o"
! CHECK14: "darwin::Link", inputs: ["{{.*}}.o"], output: "a.out"
