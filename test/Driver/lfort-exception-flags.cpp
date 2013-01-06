// RUN: %lfort -### %s 2>&1 | FileCheck %s -check-prefix=DEFAULT
// DEFAULT: "-cc1" {{.*}} "-fcxx-exceptions" "-fexceptions"
//
// RUN: %lfort -### -fexceptions %s 2>&1 | FileCheck %s -check-prefix=ON1
// ON1: "-cc1" {{.*}} "-fcxx-exceptions" "-fexceptions"
//
// RUN: %lfort -### -fno-exceptions -fcxx-exceptions %s 2>&1 | FileCheck %s -check-prefix=ON2
// ON2: "-cc1" {{.*}} "-fcxx-exceptions" "-fexceptions"
//
// RUN: %lfort -### -fno-cxx-exceptions -fexceptions %s 2>&1 | FileCheck %s -check-prefix=ON3
// ON3: "-cc1" {{.*}} "-fcxx-exceptions" "-fexceptions"
//
// RUN: %lfort -### -fno-exceptions %s 2>&1 | FileCheck %s -check-prefix=OFF1
// OFF1-NOT: "-cc1" {{.*}} "-fcxx-exceptions"
//
// RUN: %lfort -### -fno-cxx-exceptions %s 2>&1 | FileCheck %s -check-prefix=OFF2
// OFF2-NOT: "-cc1" {{.*}} "-fcxx-exceptions"
//
// RUN: %lfort -### -fcxx-exceptions -fno-exceptions %s 2>&1 | FileCheck %s -check-prefix=OFF3
// OFF3-NOT: "-cc1" {{.*}} "-fcxx-exceptions"
//
// RUN: %lfort -### -fexceptions -fno-cxx-exceptions %s 2>&1 | FileCheck %s -check-prefix=OFF4
// OFF4-NOT: "-cc1" {{.*}} "-fcxx-exceptions"
