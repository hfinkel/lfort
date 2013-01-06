// RUN: %lfort -arch x86_64 -x objective-c-header %s -o %t.h.pch
// RUN: touch %t.empty.m
// RUN: %lfort -arch x86_64 -fsyntax-only %t.empty.m -include %t.h -Xlfort -ast-dump 2>&1 > /dev/null
#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#endif

