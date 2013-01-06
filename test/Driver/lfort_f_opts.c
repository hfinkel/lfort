// RUN: %lfort -### -S -fasm -fblocks -fbuiltin -fno-math-errno -fcommon -fpascal-strings -fno-blocks -fno-builtin -fmath-errno -fno-common -fno-pascal-strings -fblocks -fbuiltin -fmath-errno -fcommon -fpascal-strings %s 2>&1 | FileCheck -check-prefix=CHECK-OPTIONS1 %s
// RUN: %lfort -### -S -fasm -fblocks -fbuiltin -fno-math-errno -fcommon -fpascal-strings -fno-asm -fno-blocks -fno-builtin -fmath-errno -fno-common -fno-pascal-strings -fno-show-source-location -fshort-enums -fshort-wchar %s 2>&1 | FileCheck -check-prefix=CHECK-OPTIONS2 %s

// CHECK-OPTIONS1: -fgnu-keywords
// CHECK-OPTIONS1: -fblocks
// CHECK-OPTIONS1: -fpascal-strings

// CHECK_OPTIONS2: -fno-gnu-keywords
// CHECK-OPTIONS2: -fmath-errno
// CHECK-OPTIONS2: -fno-builtin
// CHECK-OPTIONS2: -fshort-enums
// CHECK-OPTIONS2: -fshort-wchar
// CHECK-OPTIONS2: -fno-common
// CHECK-OPTIONS2: -fno-show-source-location

// RUN: %lfort -### -S -Wwrite-strings %s 2>&1 | FileCheck -check-prefix=WRITE-STRINGS1 %s
// WRITE-STRINGS1: -fconst-strings
// RUN: %lfort -### -S -Wwrite-strings -Wno-write-strings %s 2>&1 | FileCheck -check-prefix=WRITE-STRINGS2 %s
// WRITE-STRINGS2-NOT: -fconst-strings
// RUN: %lfort -### -S -Wwrite-strings -w %s 2>&1 | FileCheck -check-prefix=WRITE-STRINGS3 %s
// WRITE-STRINGS3: -fconst-strings

// RUN: %lfort -### -x c++ -c %s 2>&1 | FileCheck -check-prefix=DEPRECATED-ON-CHECK %s
// RUN: %lfort -### -x c++ -c -Wdeprecated %s 2>&1 | FileCheck -check-prefix=DEPRECATED-ON-CHECK %s
// RUN: %lfort -### -x c++ -c -Wno-deprecated %s 2>&1 | FileCheck -check-prefix=DEPRECATED-OFF-CHECK %s
// RUN: %lfort -### -x c++ -c -Wno-deprecated -Wdeprecated %s 2>&1 | FileCheck -check-prefix=DEPRECATED-ON-CHECK %s
// RUN: %lfort -### -x c++ -c -w %s 2>&1 | FileCheck -check-prefix=DEPRECATED-ON-CHECK %s
// RUN: %lfort -### -c %s 2>&1 | FileCheck -check-prefix=DEPRECATED-OFF-CHECK %s
// RUN: %lfort -### -c -Wdeprecated %s 2>&1 | FileCheck -check-prefix=DEPRECATED-OFF-CHECK %s
// DEPRECATED-ON-CHECK: -fdeprecated-macro
// DEPRECATED-OFF-CHECK-NOT: -fdeprecated-macro

// RUN: %lfort -### -S -ffp-contract=fast %s 2>&1 | FileCheck -check-prefix=FP-CONTRACT-FAST-CHECK %s
// RUN: %lfort -### -S -ffast-math %s 2>&1 | FileCheck -check-prefix=FP-CONTRACT-FAST-CHECK %s
// RUN: %lfort -### -S -ffp-contract=off %s 2>&1 | FileCheck -check-prefix=FP-CONTRACT-OFF-CHECK %s
// FP-CONTRACT-FAST-CHECK: -ffp-contract=fast
// FP-CONTRACT-OFF-CHECK: -ffp-contract=off

// RUN: %lfort -fms-extensions -fenable-experimental-ms-inline-asm %s -### 2>&1 | FileCheck -check-prefix=CHECK-OPTIONS3 %s
// CHECK-OPTIONS3: -fenable-experimental-ms-inline-asm

// RUN: %lfort -### -S -fvectorize %s 2>&1 | FileCheck -check-prefix=CHECK-VECTORIZE %s
// RUN: %lfort -### -S -fno-vectorize -fvectorize %s 2>&1 | FileCheck -check-prefix=CHECK-VECTORIZE %s
// RUN: %lfort -### -S -fno-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-VECTORIZE %s
// RUN: %lfort -### -S -fvectorize -fno-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-VECTORIZE %s
// RUN: %lfort -### -S -ftree-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-VECTORIZE %s
// RUN: %lfort -### -S -fno-tree-vectorize -fvectorize %s 2>&1 | FileCheck -check-prefix=CHECK-VECTORIZE %s
// RUN: %lfort -### -S -fno-tree-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-VECTORIZE %s
// RUN: %lfort -### -S -ftree-vectorize -fno-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-VECTORIZE %s
// CHECK-VECTORIZE: "-vectorize-loops"
// CHECK-NO-VECTORIZE-NOT: "-vectorize-loops"

// RUN: %lfort -### -S -fslp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-SLP-VECTORIZE %s
// RUN: %lfort -### -S -fno-slp-vectorize -fslp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-SLP-VECTORIZE %s
// RUN: %lfort -### -S -fno-slp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-SLP-VECTORIZE %s
// RUN: %lfort -### -S -fslp-vectorize -fno-slp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-SLP-VECTORIZE %s
// RUN: %lfort -### -S -ftree-slp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-SLP-VECTORIZE %s
// RUN: %lfort -### -S -fno-tree-slp-vectorize -fslp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-SLP-VECTORIZE %s
// RUN: %lfort -### -S -fno-tree-slp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-SLP-VECTORIZE %s
// RUN: %lfort -### -S -ftree-slp-vectorize -fno-slp-vectorize %s 2>&1 | FileCheck -check-prefix=CHECK-NO-SLP-VECTORIZE %s
// CHECK-SLP-VECTORIZE: "-vectorize"
// CHECK-NO-SLP-VECTORIZE-NOT: "-vectorize"
