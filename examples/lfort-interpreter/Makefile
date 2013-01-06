##===- examples/lfort-interpreter/Makefile -----------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ../..

TOOLNAME = lfort-interpreter
NO_INSTALL = 1

# No plugins, optimize startup time.
TOOL_NO_EXPORTS = 1

LINK_COMPONENTS := jit interpreter nativecodegen bitreader bitwriter ipo \
	linker selectiondag asmparser instrumentation
USEDLIBS = lfortFrontend.a lfortSerialization.a lfortDriver.a lfortCodeGen.a \
           lfortParse.a lfortSema.a lfortStaticAnalyzerFrontend.a \
           lfortStaticAnalyzerCheckers.a lfortStaticAnalyzerCore.a \
           lfortAnalysis.a lfortRewriteCore.a lfortRewriteFrontend.a \
           lfortEdit.a lfortAST.a lfortLex.a lfortBasic.a

include $(LFORT_LEVEL)/Makefile
