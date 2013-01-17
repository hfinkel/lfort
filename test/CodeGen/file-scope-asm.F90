! RUN: %lfort_cc1 %s -triple x86_64-apple-darwin -emit-llvm -o - | FileCheck %s
asm (                  &
    "my_function:\n\t" &
    "jmp *%eax\n\t"    &
)
! CHECK: module asm "my_function:"
! CHECK: module asm "\09jmp *%eax"
! CHECK: module asm "\09"

program hello
end program hello
! CHECK: @MAIN__

