// RUN: rm -rf %t
// RUN: %lfort_cc1 -fmodules -fmodule-cache-path %t -F %S/Inputs -I %S/Inputs -verify %s
// expected-no-diagnostics

#import "point.h"
@import Module;
#import "point.h"

