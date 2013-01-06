##===- unittests/Format/Makefile ---------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL = ../..
TESTNAME = Format
include $(LFORT_LEVEL)/../../Makefile.config
LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser support mc
USEDLIBS = lfortFormat.a lfortTooling.a lfortFrontend.a lfortSerialization.a \
           lfortDriver.a lfortParse.a lfortRewriteCore.a \
           lfortRewriteFrontend.a lfortSema.a lfortAnalysis.a lfortEdit.a \
           lfortAST.a lfortASTMatchers.a lfortLex.a lfortBasic.a

include $(LFORT_LEVEL)/unittests/Makefile
