// RUN: %lfort_cc1 -triple thumbv7-apple-darwin10 -target-cpu cortex-a8 -fsyntax-only -Wvector-conversions -ffreestanding %s
// RUN: %lfort_cc1 -triple thumbv7-apple-darwin10 -target-cpu cortex-a8 -fsyntax-only -fno-lax-vector-conversions -ffreestanding %s
// RUN: %lfort_cc1 -x c++ -triple thumbv7-apple-darwin10 -target-cpu cortex-a8 -fsyntax-only -Wvector-conversions -ffreestanding %s

#include <arm_neon.h>
