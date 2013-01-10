! RUN: %lfort --help | grep isystem
! RUN: %lfort --help | not grep ast-dump
! RUN: %lfort --help | not grep ccc-f77
! RUN: %lfort --help-hidden | grep ccc-f77
! RUN: %lfort -dumpversion
! RUN: %lfort -print-search-dirs
