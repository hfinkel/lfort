// RUN: rm -rf %t
// RUN: %lfort -fsyntax-only -fmodules -fmodule-cache-path %t -D__need_wint_t %s -Xlfort -verify
// RUN: %lfort -fsyntax-only -std=c99 -fmodules -fmodule-cache-path %t -D__need_wint_t %s -Xlfort -verify
// expected-no-diagnostics

#ifdef __SSE__
@import _Builtin_intrinsics.intel.sse;
#endif

#ifdef __AVX2__
@import _Builtin_intrinsics.intel.avx2;
#endif
