// RUN: %lfort -fmodules -### %s 2>&1 | FileCheck -check-prefix=CHECK-NO-MODULES %s
// RUN: %lfort -fcxx-modules -### %s 2>&1 | FileCheck -check-prefix=CHECK-NO-MODULES %s
// CHECK-NO-MODULES-NOT: -fmodules

// RUN: %lfort -fcxx-modules -fmodules -### %s 2>&1 | FileCheck -check-prefix=CHECK-HAS-MODULES %s
// CHECK-HAS-MODULES: -fmodules
