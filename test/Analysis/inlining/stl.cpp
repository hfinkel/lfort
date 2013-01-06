// RUN: %lfort_cc1 -analyze -analyzer-checker=core,unix.Malloc,debug.ExprInspection -analyzer-ipa=dynamic -analyzer-config c++-stdlib-inlining=false -verify %s
// RUN: %lfort_cc1 -analyze -analyzer-checker=core,unix.Malloc,debug.ExprInspection -analyzer-ipa=dynamic -analyzer-config c++-stdlib-inlining=true -DINLINE=1 -verify %s

#include "../Inputs/system-header-simulator-cxx.h"

void lfort_analyzer_eval(bool);

void testVector(std::vector<int> &nums) {
  if (nums.begin()) return;
  if (nums.end()) return;
  
  lfort_analyzer_eval(nums.size() == 0);
#if INLINE
  // expected-warning@-2 {{TRUE}}
#else
  // expected-warning@-4 {{UNKNOWN}}
#endif
}

void testException(std::exception e) {
  // Notice that the argument is NOT passed by reference, so we can devirtualize.
  const char *x = e.what();
  lfort_analyzer_eval(x == 0);
#if INLINE
  // expected-warning@-2 {{TRUE}}
#else
  // expected-warning@-4 {{UNKNOWN}}
#endif
}
