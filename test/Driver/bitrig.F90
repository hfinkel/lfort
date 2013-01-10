! RUN: %lfort -no-canonical-prefixes -target amd64-pc-bitrig %s -### 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-LD-C %s
! CHECK-LD-C: lfort{{.*}}" "-cc1" "-triple" "amd64-pc-bitrig"
! CHECK-LD-C: ld{{.*}}" {{.*}} "-lc" "-llfort_rt.amd64"

! FIXME: Add tests for finding the C++ runtime if necessary.

! RUN: %lfort -no-canonical-prefixes -target amd64-pc-bitrig -pthread %s -### 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-PTHREAD %s
! CHECK-PTHREAD: lfort{{.*}}" "-cc1" "-triple" "amd64-pc-bitrig"
! CHECK-PTHREAD: ld{{.*}}" {{.*}} "{{.*}}crtbegin.o" {{.*}}.o" "-lfortrt" "-lm" "-lpthread" "-lc" "-llfort_rt.amd64" "{{.*}}crtend.o"

! RUN: %lfort -no-canonical-prefixes -target amd64-pc-bitrig -pg -pthread %s -### 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-PG-PTHREAD %s
! CHECK-PG-PTHREAD: lfort{{.*}}" "-cc1" "-triple" "amd64-pc-bitrig"
! CHECK-PG-PTHREAD: ld{{.*}}" {{.*}} "{{.*}}crtbegin.o" {{.*}}.o" "-lfortrt" "-lm_p" "-lpthread_p" "-lc_p" "-llfort_rt.amd64" "{{.*}}crtend.o"

! RUN: %lfort -no-canonical-prefixes -target amd64-pc-bitrig -shared -pg -pthread %s -### 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-PG-PTHREAD-SHARED %s
! CHECK-PG-PTHREAD-SHARED: lfort{{.*}}" "-cc1" "-triple" "amd64-pc-bitrig"
! CHECK-PG-PTHREAD-SHARED: ld{{.*}}" {{.*}} "{{.*}}crtbeginS.o" {{.*}}.o" "-lfortrt" "-lm_p" "-lpthread" "-llfort_rt.amd64" "{{.*}}crtendS.o"
