##===- tools/diagtool/Makefile -----------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LFORT_LEVEL := ../..

TOOLNAME = diagtool 

# No plugins, optimize startup time.
TOOL_NO_EXPORTS := 1

# Don't install this.
NO_INSTALL = 1

include $(LFORT_LEVEL)/../../Makefile.config
LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser support mc
USEDLIBS = lfortFrontend.a lfortDriver.a lfortSerialization.a lfortParse.a \
           lfortSema.a lfortAnalysis.a lfortEdit.a lfortAST.a lfortLex.a \
           lfortBasic.a

include $(LFORT_LEVEL)/Makefile

