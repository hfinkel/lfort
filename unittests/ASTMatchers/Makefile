##===- unittests/ASTMatchers/Makefile ----------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL = ../..

TESTNAME = ASTMatchers
include $(LFORT_LEVEL)/../../Makefile.config
LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser support mc
USEDLIBS = lfortTooling.a lfortFrontend.a lfortSerialization.a lfortDriver.a \
           lfortRewriteCore.a lfortRewriteFrontend.a \
           lfortParse.a lfortSema.a lfortAnalysis.a \
           lfortEdit.a lfortAST.a lfortASTMatchers.a lfortLex.a lfortBasic.a

include $(LFORT_LEVEL)/unittests/Makefile
