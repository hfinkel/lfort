! RUN: %lfort -target i386-unknown-unknown -mno-red-zone %s -S -emit-llvm -o %t.log
! RUN: grep 'noredzone' %t.log
! RUN: %lfort -target i386-unknown-unknown -mred-zone %s -S -emit-llvm -o %t.log
! RUN: grep -v 'noredzone' %t.log 

end
