##===- tools/c-arcmt-test/Makefile -------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
LFORT_LEVEL := ../..

TOOLNAME = c-arcmt-test

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

# Don't install this. It is used for tests.
NO_INSTALL = 1

# Include this here so we can get the configuration of the targets that have
# been configured for construction. We have to do this early so we can set up
# LINK_COMPONENTS before including Makefile.rules
include $(LFORT_LEVEL)/../../Makefile.config

LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser support mc

# Note that 'USEDLIBS' must include all of the core lfort libraries
# as lfort.dll is unavailable on cygming yet.
USEDLIBS = lfort.a \
	   lfortARCMigrate.a \
	   lfortRewriteFrontend.a \
	   lfortRewriteCore.a \
	   lfortFrontend.a lfortDriver.a \
	   lfortSerialization.a lfortParse.a lfortSema.a \
	   lfortAnalysis.a lfortEdit.a lfortAST.a lfortLex.a lfortBasic.a

include $(LFORT_LEVEL)/Makefile
