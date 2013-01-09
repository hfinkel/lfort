! RUN: %lfort --help | grep isystem
! RUN: %lfort --help | not grep ast-dump
! RUN: %lfort --help | not grep ccc-cxx
! RUN: %lfort --help-hidden | grep ccc-cxx
! RUN: %lfort -dumpversion
! RUN: %lfort -print-search-dirs
