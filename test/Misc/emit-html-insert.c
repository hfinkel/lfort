// RUN: %lfort_cc1 %s -emit-html -o - | grep ">&lt; 10; }"

int a(int x) { return x
< 10; }
