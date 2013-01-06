##===- lfort/lib/FrontendTool/Makefile ---------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ../..
LIBRARYNAME := lfortFrontendTool

include $(LFORT_LEVEL)/Makefile
include $(LFORT_LEVEL)/../../Makefile.config

ifeq ($(ENABLE_LFORT_REWRITER),1)
  CXX.Flags += -DLFORT_ENABLE_REWRITER
endif

ifeq ($(ENABLE_LFORT_STATIC_ANALYZER),1)
  CXX.Flags += -DLFORT_ENABLE_STATIC_ANALYZER
endif


