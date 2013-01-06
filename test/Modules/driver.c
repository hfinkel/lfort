// RUN: %lfort %s -### 2>&1 | FileCheck -check-prefix NO_MODULE_CACHE %s
// RUN: %lfort -fmodule-cache-path blarg %s -### 2>&1 | FileCheck -check-prefix WITH_MODULE_CACHE %s

// CHECK-NO_MODULE_CACHE: {{lfort.*"-fmodule-cache-path"}}

// CHECK-WITH_MODULE_CACHE: {{lfort.*"-fmodule-cache-path" "blarg"}}
