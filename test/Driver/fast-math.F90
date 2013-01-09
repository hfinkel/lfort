! Test that the GCC fast-math floating point flags get lowered to the correct
! permutation of LFort frontend flags. This is non-trivial for a few reasons.
! First, the GCC flags have many different and surprising effects. Second,
! LLVM only supports three switches which is more coarse grained than GCC's
! support.
!
! Both of them use gcc driver for as.
! REQUIRES: lfort-driver
!
! RUN: %lfort -### -fno-honor-infinities -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-INFS %s
! CHECK-NO-INFS: "-cc1"
! CHECK-NO-INFS: "-menable-no-infs"
!
! RUN: %lfort -### -fno-fast-math -fno-honor-infinities -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-FAST-MATH-NO-INFS %s
! CHECK-NO-FAST-MATH-NO-INFS: "-cc1"
! CHECK-NO-FAST-MATH-NO-INFS: "-menable-no-infs"
!
! RUN: %lfort -### -fno-honor-infinities -fno-fast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-INFS-NO-FAST-MATH %s
! CHECK-NO-INFS-NO-FAST-MATH: "-cc1"
! CHECK-NO-INFS-NO-FAST-MATH-NOT: "-menable-no-infs"
!
! RUN: %lfort -### -fno-honor-nans -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NANS %s
! CHECK-NO-NANS: "-cc1"
! CHECK-NO-NANS: "-menable-no-nans"
!
! RUN: %lfort -### -fno-fast-math -fno-honor-nans -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-FAST-MATH-NO-NANS %s
! CHECK-NO-FAST-MATH-NO-NANS: "-cc1"
! CHECK-NO-FAST-MATH-NO-NANS: "-menable-no-nans"
!
! RUN: %lfort -### -fno-honor-nans -fno-fast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NANS-NO-FAST-MATH %s
! CHECK-NO-NANS-NO-FAST-MATH: "-cc1"
! CHECK-NO-NANS-NO-FAST-MATH-NOT: "-menable-no-nans"
!
! RUN: %lfort -### -fmath-errno -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-MATH-ERRNO %s
! CHECK-MATH-ERRNO: "-cc1"
! CHECK-MATH-ERRNO: "-fmath-errno"
!
! RUN: %lfort -### -fno-fast-math -fmath-errno -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-FAST-MATH-MATH-ERRNO %s
! CHECK-NO-FAST-MATH-MATH-ERRNO: "-cc1"
! CHECK-NO-FAST-MATH-MATH-ERRNO: "-fmath-errno"
!
! RUN: %lfort -### -fmath-errno -fno-fast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-MATH-ERRNO-NO-FAST-MATH %s
! CHECK-MATH-ERRNO-NO-FAST-MATH: "-cc1"
! CHECK-MATH-ERRNO-NO-FAST-MATH-NOT: "-fmath-errno"
!
! RUN: %lfort -### -fmath-errno -fno-math-errno -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-MATH-ERRNO %s
! RUN: %lfort -### -target i686-apple-darwin -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-MATH-ERRNO %s
! RUN: %lfort -### -target x86_64-unknown-freebsd -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-MATH-ERRNO %s
! RUN: %lfort -### -target x86_64-unknown-netbsd -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-MATH-ERRNO %s
! RUN: %lfort -### -target x86_64-unknown-openbsd -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-MATH-ERRNO %s
! RUN: %lfort -### -target x86_64-unknown-dragonfly -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-MATH-ERRNO %s
! CHECK-NO-MATH-ERRNO: "-cc1"
! CHECK-NO-MATH-ERRNO-NOT: "-fmath-errno"
!
! RUN: %lfort -### -fno-math-errno -fassociative-math -freciprocal-math \
! RUN:     -fno-signed-zeros -fno-trapping-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-UNSAFE-MATH %s
! CHECK-UNSAFE-MATH: "-cc1"
! CHECK-UNSAFE-MATH: "-menable-unsafe-fp-math"
!
! RUN: %lfort -### -fno-fast-math -fno-math-errno -fassociative-math -freciprocal-math \
! RUN:     -fno-signed-zeros -fno-trapping-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-FAST-MATH-UNSAFE-MATH %s
! CHECK-NO-FAST-MATH-UNSAFE-MATH: "-cc1"
! CHECK-NO-FAST-MATH-UNSAFE-MATH: "-menable-unsafe-fp-math"
!
! RUN: %lfort -### -fno-fast-math -fno-math-errno -fassociative-math -freciprocal-math \
! RUN:     -fno-fast-math -fno-signed-zeros -fno-trapping-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-UNSAFE-MATH-NO-FAST-MATH %s
! CHECK-UNSAFE-MATH-NO-FAST-MATH: "-cc1"
! CHECK-UNSAFE-MATH-NO-FAST-MATH-NOT: "-menable-unsafe-fp-math"
!
! Check that various umbrella flags also enable these frontend options.
! RUN: %lfort -### -ffast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-INFS %s
! RUN: %lfort -### -ffast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NANS %s
! RUN: %lfort -### -ffast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-UNSAFE-MATH %s
! RUN: %lfort -### -ffinite-math-only -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-INFS %s
! RUN: %lfort -### -ffinite-math-only -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NANS %s
! RUN: %lfort -### -funsafe-math-optimizations -fno-math-errno -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-UNSAFE-MATH %s
!
! One umbrella flag is *really* weird and also changes the semantics of the
! program by adding a special preprocessor macro. Check that the frontend flag
! modeling this semantic change is provided. Also check that the semantic
! impact remains even if every optimization is disabled.
! RUN: %lfort -### -ffast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-FAST-MATH %s
! RUN: %lfort -### -fno-fast-math -ffast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-FAST-MATH %s
! RUN: %lfort -### -ffast-math -fno-finite-math-only \
! RUN:     -fno-unsafe-math-optimizations -fmath-errno -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-FAST-MATH %s
! CHECK-FAST-MATH: "-cc1"
! CHECK-FAST-MATH: "-ffast-math"
!
! RUN: %lfort -### -ffast-math -fno-fast-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-FAST-MATH %s
! CHECK-NO-FAST-MATH: "-cc1"
! CHECK-NO-FAST-MATH-NOT: "-ffast-math"
!
! Check various means of disabling these flags, including disabling them after
! they've been enabled via an umbrella flag.
! RUN: %lfort -### -fno-honor-infinities -fhonor-infinities -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-INFS %s
! RUN: %lfort -### -ffinite-math-only -fhonor-infinities -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-INFS %s
! RUN: %lfort -### -ffinite-math-only -fno-finite-math-only -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-INFS %s
! RUN: %lfort -### -ffast-math -fhonor-infinities -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-INFS %s
! RUN: %lfort -### -ffast-math -fno-finite-math-only -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-INFS %s
! CHECK-NO-NO-INFS: "-cc1"
! CHECK-NO-NO-INFS-NOT: "-menable-no-infs"
! CHECK-NO-NO-INFS: "-o"
!
! RUN: %lfort -### -fno-honor-nans -fhonor-nans -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-NANS %s
! RUN: %lfort -### -ffinite-math-only -fhonor-nans -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-NANS %s
! RUN: %lfort -### -ffinite-math-only -fno-finite-math-only -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-NANS %s
! RUN: %lfort -### -ffast-math -fhonor-nans -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-NANS %s
! RUN: %lfort -### -ffast-math -fno-finite-math-only -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-NO-NANS %s
! CHECK-NO-NO-NANS: "-cc1"
! CHECK-NO-NO-NANS-NOT: "-menable-no-nans"
! CHECK-NO-NO-NANS: "-o"
!
! RUN: %lfort -### -fassociative-math -freciprocal-math -fno-signed-zeros \
! RUN:     -fno-trapping-math -fno-associative-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -fassociative-math -freciprocal-math -fno-signed-zeros \
! RUN:     -fno-trapping-math -fno-reciprocal-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -fassociative-math -freciprocal-math -fno-signed-zeros \
! RUN:     -fno-trapping-math -fsigned-zeros -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -fassociative-math -freciprocal-math -fno-signed-zeros \
! RUN:     -fno-trapping-math -ftrapping-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -funsafe-math-optimizations -fno-associative-math -c %s \
! RUN:   2>&1 | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -funsafe-math-optimizations -fno-reciprocal-math -c %s \
! RUN:   2>&1 | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -funsafe-math-optimizations -fsigned-zeros -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -funsafe-math-optimizations -ftrapping-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -funsafe-math-optimizations -fno-unsafe-math-optimizations \
! RUN:     -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -ffast-math -fno-associative-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -ffast-math -fno-reciprocal-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -ffast-math -fsigned-zeros -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -ffast-math -ftrapping-math -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! RUN: %lfort -### -ffast-math -fno-unsafe-math-optimizations -c %s 2>&1 \
! RUN:   | FileCheck --check-prefix=CHECK-NO-UNSAFE-MATH %s
! CHECK-NO-UNSAFE-MATH: "-cc1"
! CHECK-NO-UNSAFE-MATH-NOT: "-menable-unsafe-fp-math"
! CHECK-NO-UNSAFE-MATH: "-o"
