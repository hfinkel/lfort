// RUN: %lfort_cc1 -triple i386-unknown-unknown -emit-pch -o %t1 %S/pchpch1.h
// RUN: %lfort_cc1 -triple i386-unknown-unknown -emit-pch -o %t2 %S/pchpch2.h -include-pch %t1
// RUN: %lfort_cc1 -triple i386-unknown-unknown -fsyntax-only %s -include-pch %t2

// The purpose of this test is to make sure that a PCH created while including
// an existing PCH can be loaded.
