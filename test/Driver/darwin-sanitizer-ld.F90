! Test sanitizer link flags on Darwin.

! RUN: %lfort -no-canonical-prefixes -### -target x86_64-darwin \
! RUN:   -fsanitize=address %s -o %t.o 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-ASAN %s

! CHECK-ASAN: "{{.*}}ld{{(.exe)?}}"
! CHECK-ASAN: liblfort_rt.asan_osx.a"
! CHECK-ASAN: stdc++
! CHECK-ASAN: "-framework" "CoreFoundation"

! RUN: %lfort -no-canonical-prefixes -### -target x86_64-darwin \
! RUN:   -fPIC -shared -fsanitize=address %s -o %t.so 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-DYN-ASAN %s

! CHECK-DYN-ASAN: "{{.*}}ld{{(.exe)?}}"
! CHECK-DYN-ASAN: "-dylib"
! CHECK-DYN-ASAN-NOT: liblfort_rt.asan_osx.a
! CHECK-DYN-ASAN: "-undefined"
! CHECK-DYN-ASAN: "dynamic_lookup"
! CHECK-DYN-ASAN-NOT: liblfort_rt.asan_osx.a

! RUN: %lfort -no-canonical-prefixes -### -target x86_64-darwin \
! RUN:   -fsanitize=undefined %s -o %t.o 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-UBSAN %s

! CHECK-UBSAN: "{{.*}}ld{{(.exe)?}}"
! CHECK-UBSAN: liblfort_rt.ubsan_osx.a"
! CHECK-UBSAN: stdc++

! RUN: %lfort -no-canonical-prefixes -### -target x86_64-darwin \
! RUN:   -fsanitize=bounds %s -o %t.o 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-BOUNDS %s

! CHECK-BOUNDS: "{{.*}}ld{{(.exe)?}}"
! CHECK-BOUNDS-NOT: liblfort_rt.ubsan_osx.a"

! RUN: %lfort -no-canonical-prefixes -### -target x86_64-darwin \
! RUN:   -fPIC -shared -fsanitize=undefined %s -o %t.so 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-DYN-UBSAN %s

! CHECK-DYN-UBSAN: "{{.*}}ld{{(.exe)?}}"
! CHECK-DYN-UBSAN: "-dylib"
! CHECK-DYN-UBSAN-NOT: liblfort_rt.ubsan_osx.a
! CHECK-DYN-UBSAN: "-undefined"
! CHECK-DYN-UBSAN: "dynamic_lookup"
! CHECK-DYN-UBSAN-NOT: liblfort_rt.ubsan_osx.a

! RUN: %lfort -no-canonical-prefixes -### -target x86_64-darwin \
! RUN:   -fPIC -shared -fsanitize=bounds %s -o %t.so 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-DYN-BOUNDS %s

! CHECK-DYN-BOUNDS: "{{.*}}ld{{(.exe)?}}"
! CHECK-DYN-BOUNDS-NOT: liblfort_rt.ubsan_osx.a
