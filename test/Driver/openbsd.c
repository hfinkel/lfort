// RUN: %lfort -no-canonical-prefixes -target i686-pc-openbsd %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-LD %s
// CHECK-LD: lfort{{.*}}" "-cc1" "-triple" "i686-pc-openbsd"
// CHECK-LD: ld{{.*}}" "-e" "__start" "--eh-frame-hdr" "-Bdynamic" "-dynamic-linker" "{{.*}}ld.so" "-o" "a.out" "{{.*}}crt0.o" "{{.*}}crtbegin.o" "{{.*}}.o" "-lgcc" "-lc" "-lgcc" "{{.*}}crtend.o"

// RUN: %lfort -no-canonical-prefixes -target i686-pc-openbsd -pg -pthread %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-PG %s
// CHECK-PG: lfort{{.*}}" "-cc1" "-triple" "i686-pc-openbsd"
// CHECK-PG: ld{{.*}}" "-e" "__start" "--eh-frame-hdr" "-Bdynamic" "-dynamic-linker" "{{.*}}ld.so" "-o" "a.out" "{{.*}}crt0.o" "{{.*}}crtbegin.o" "{{.*}}.o" "-lgcc" "-lpthread_p" "-lc_p" "-lgcc" "{{.*}}crtend.o"

// Check that the new linker flags are passed to OpenBSD
// RUN: %lfort -no-canonical-prefixes -target i686-pc-openbsd -r %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-LD-R %s
// RUN: %lfort -no-canonical-prefixes -target i686-pc-openbsd -s %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-LD-S %s
// RUN: %lfort -no-canonical-prefixes -target i686-pc-openbsd -t %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-LD-T %s
// RUN: %lfort -no-canonical-prefixes -target i686-pc-openbsd -Z %s -### 2>&1 \
// RUN:   | FileCheck --check-prefix=CHECK-LD-Z %s
// CHECK-LD-R: lfort{{.*}}" "-cc1" "-triple" "i686-pc-openbsd"
// CHECK-LD-R: ld{{.*}}" "-e" "__start" "--eh-frame-hdr" "-Bdynamic" "-dynamic-linker" "{{.*}}ld.so" "-o" "a.out" "{{.*}}crt0.o" "{{.*}}crtbegin.o" "-L{{.*}}" "-r" "{{.*}}.o" "-lgcc" "-lc" "-lgcc" "{{.*}}crtend.o"
// CHECK-LD-S: lfort{{.*}}" "-cc1" "-triple" "i686-pc-openbsd"
// CHECK-LD-S: ld{{.*}}" "-e" "__start" "--eh-frame-hdr" "-Bdynamic" "-dynamic-linker" "{{.*}}ld.so" "-o" "a.out" "{{.*}}crt0.o" "{{.*}}crtbegin.o" "-L{{.*}}" "-s" "{{.*}}.o" "-lgcc" "-lc" "-lgcc" "{{.*}}crtend.o"
// CHECK-LD-T: lfort{{.*}}" "-cc1" "-triple" "i686-pc-openbsd"
// CHECK-LD-T: ld{{.*}}" "-e" "__start" "--eh-frame-hdr" "-Bdynamic" "-dynamic-linker" "{{.*}}ld.so" "-o" "a.out" "{{.*}}crt0.o" "{{.*}}crtbegin.o" "-L{{.*}}" "-t" "{{.*}}.o" "-lgcc" "-lc" "-lgcc" "{{.*}}crtend.o"
// CHECK-LD-Z: lfort{{.*}}" "-cc1" "-triple" "i686-pc-openbsd"
// CHECK-LD-Z: ld{{.*}}" "-e" "__start" "--eh-frame-hdr" "-Bdynamic" "-dynamic-linker" "{{.*}}ld.so" "-o" "a.out" "{{.*}}crt0.o" "{{.*}}crtbegin.o" "-L{{.*}}" "-Z" "{{.*}}.o" "-lgcc" "-lc" "-lgcc" "{{.*}}crtend.o"
