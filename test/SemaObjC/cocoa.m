// RUN: %lfort -arch x86_64 %s -fsyntax-only -Xlfort -print-stats 
#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#endif

