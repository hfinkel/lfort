##===- lfort/lib/Checker/Makefile --------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
#
# This implements analyses built on top of source-level CFGs. 
#
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ../../..
LIBRARYNAME := lfortStaticAnalyzerCheckers

BUILT_SOURCES = Checkers.inc
TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

$(ObjDir)/Checkers.inc.tmp : Checkers.td $(PROJ_SRC_DIR)/$(LFORT_LEVEL)/include/lfort/StaticAnalyzer/Checkers/CheckerBase.td $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort SA Checkers tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-sa-checkers -I $(PROJ_SRC_DIR)/$(LFORT_LEVEL)/include -o $(call SYSPATH, $@) $<
