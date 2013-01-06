##===- lfort/lib/Lex/Makefile ------------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
#
#  This implements the Lexer library for the C-Language front-end.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ../..
include $(LFORT_LEVEL)/../../Makefile.config

LIBRARYNAME := lfortLex

ifeq ($(ARCH),PowerPC)
CXX.Flags += -maltivec
endif

include $(LFORT_LEVEL)/Makefile

