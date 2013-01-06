##===- unittests/Makefile ----------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# If LFORT_LEVEL is not set, then we are the top-level Makefile. Otherwise, we
# are being included from a subdirectory makefile.

ifndef LFORT_LEVEL

IS_UNITTEST_LEVEL := 1
LFORT_LEVEL := ..
PARALLEL_DIRS = Basic Lex

include $(LFORT_LEVEL)/../..//Makefile.config

ifeq ($(ENABLE_LFORT_REWRITER),1)
PARALLEL_DIRS += Format
endif

ifeq ($(ENABLE_LFORT_REWRITER),1)
PARALLEL_DIRS += ASTMatchers AST Tooling
endif

ifeq ($(ENABLE_LFORT_STATIC_ANALYZER),1)
PARALLEL_DIRS += Frontend
endif

endif  # LFORT_LEVEL

include $(LFORT_LEVEL)/Makefile

ifndef IS_UNITTEST_LEVEL

MAKEFILE_UNITTEST_NO_INCLUDE_COMMON := 1
include $(LLVM_SRC_ROOT)/unittests/Makefile.unittest

endif  # IS_UNITTEST_LEVEL
