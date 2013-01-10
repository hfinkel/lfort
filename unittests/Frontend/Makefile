##===- unittests/Frontend/Makefile -------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL = ../..
TESTNAME = Frontend
include $(LFORT_LEVEL)/../../Makefile.config
LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser support mc
USEDLIBS = lfortFrontendTool.a lfortFrontend.a lfortDriver.a \
           lfortSerialization.a lfortCodeGen.a lfortParse.a lfortSema.a \
           lfortStaticAnalyzerCheckers.a lfortStaticAnalyzerCore.a \
           lfortRewriteCore.a \
		   lfortRewriteFrontend.a lfortEdit.a \
           lfortAnalysis.a lfortAST.a lfortLex.a lfortBasic.a

include $(LFORT_LEVEL)/unittests/Makefile
