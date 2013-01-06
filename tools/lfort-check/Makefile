##===- tools/lfort-check/Makefile --------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ../..

TOOLNAME = lfort-check

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

include $(LFORT_LEVEL)/../../Makefile.config
LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser support mc
USEDLIBS = lfortFrontend.a lfortSerialization.a lfortDriver.a \
           lfortTooling.a lfortParse.a lfortSema.a lfortAnalysis.a \
           lfortRewriteFrontend.a lfortRewriteCore.a lfortEdit.a lfortAST.a \
           lfortLex.a lfortBasic.a

include $(LFORT_LEVEL)/Makefile
