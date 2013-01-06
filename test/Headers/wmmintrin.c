// Check that wmmintrin.h is includable with just -maes.
// RUN: %lfort_cc1 -triple x86_64-unknown-unknown \
// RUN:   -verify %s -ffreestanding -target-feature +aes
// expected-no-diagnostics
#include <wmmintrin.h>
