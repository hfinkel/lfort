// RUN: %lfort  -Werror -fobjc-arc -fsyntax-only -fno-objc-arc -Xlfort -verify %s
// expected-no-diagnostics
// rdar://8949617

void * FOO() {
    id string = @"Hello World.\n";
    void *pointer = string; // No error must be issued
    return pointer;
}
