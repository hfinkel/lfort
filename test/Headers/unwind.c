// RUN: %lfort_cc1 -triple arm-unknown-linux-gnueabi \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only %s
// RUN: %lfort_cc1 -triple mips-unknown-linux \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only %s
// RUN: %lfort_cc1 -triple i686-unknown-linux \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only %s
// RUN: %lfort_cc1 -triple x86_64-unknown-linux \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only %s

// RUN: %lfort_cc1 -triple arm-unknown-linux-gnueabi \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only -x c++ %s
// RUN: %lfort_cc1 -triple mips-unknown-linux \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only -x c++ %s
// RUN: %lfort_cc1 -triple i686-unknown-linux \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only -x c++ %s
// RUN: %lfort_cc1 -triple x86_64-unknown-linux \
// RUN:   -isystem %S/Inputs/include -ffreestanding -fsyntax-only -x c++ %s

#include "unwind.h"
