// RUN: mkdir -p %t.h.gch
// RUN: %lfort -x c-header %S/pch-dir.h -DFOO=foo -o %t.h.gch/c.gch 
// RUN: %lfort -x c-header %S/pch-dir.h -DFOO=bar -o %t.h.gch/cbar.gch 
// RUN: %lfort -x c++-header -std=c++98 %S/pch-dir.h -o %t.h.gch/cpp.gch 
// RUN: %lfort -include %t.h -DFOO=foo -fsyntax-only %s -Xlfort -print-stats 2> %t.clog
// RUN: FileCheck -check-prefix=C %s < %t.clog
// RUN: %lfort -include %t.h -DFOO=bar -DBAR=bar -fsyntax-only %s -Xlfort -ast-print > %t.cbarlog
// RUN: FileCheck -check-prefix=CBAR %s < %t.cbarlog
// RUN: %lfort -x c++ -include %t.h -std=c++98 -fsyntax-only %s -Xlfort -print-stats 2> %t.cpplog
// RUN: FileCheck -check-prefix=CPP %s < %t.cpplog

// RUN: not %lfort -x c++ -std=c++11 -include %t.h -fsyntax-only %s 2> %t.cpp11log
// RUN: FileCheck -check-prefix=CPP11 %s < %t.cpp11log

// CHECK-CBAR: int bar
int FOO;

int get() {
#ifdef __cplusplus
  // CHECK-CPP: .h.gch{{[/\\]}}cpp.gch
  return i;
#else
  // CHECK-C: .h.gch{{[/\\]}}c.gch
  return j;
#endif
}

// CHECK-CPP11: no suitable precompiled header file found in directory
