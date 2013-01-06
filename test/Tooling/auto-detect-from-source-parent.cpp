// RUN: rm -rf %t
// RUN: mkdir -p %t/abc/def/ijk/qwe
// RUN: echo "[{\"directory\":\".\",\"command\":\"lfort++ -c %t/abc/def/ijk/qwe/test.cpp\",\"file\":\"%t/abc/def/ijk/qwe/test.cpp\"}]" | sed -e 's/\\/\\\\/g' > %t/compile_commands.json
// RUN: cp "%s" "%t/abc/def/ijk/qwe/test.cpp"
// RUN: lfort-check "%t/abc/def/ijk/qwe/test.cpp" 2>&1 | FileCheck %s

// CHECK: C++ requires
invalid;

// REQUIRES: shell
