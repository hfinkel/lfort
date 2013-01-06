// RUN: %lfort -no-canonical-prefixes -target amd64-pc-dragonfly %s -### 2> %t.log
// RUN: FileCheck -input-file %t.log %s

// CHECK: lfort{{.*}}" "-cc1" "-triple" "amd64-pc-dragonfly"
// CHECK: ld{{.*}}" "-dynamic-linker" "{{.*}}ld-elf.{{.*}}" "-o" "a.out" "{{.*}}crt1.o" "{{.*}}crti.o" "{{.*}}crtbegin.o" "{{.*}}.o" "-L{{.*}}/gcc{{.*}}" {{.*}} "-lc" "-lgcc" "{{.*}}crtend.o" "{{.*}}crtn.o"


