##===- tools/Makefile --------------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ..

include $(LFORT_LEVEL)/../../Makefile.config

DIRS := driver liblfort c-index-test diagtool \
        lfort-check

# Recurse into the extra repository of tools if present.
OPTIONAL_DIRS := extra

ifeq ($(BUILD_LFORT_ONLY),YES)
  DIRS := driver liblfort c-index-test
  OPTIONAL_DIRS :=
endif

include $(LFORT_LEVEL)/Makefile
