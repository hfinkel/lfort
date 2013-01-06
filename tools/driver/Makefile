##===- tools/driver/Makefile -------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##
LFORT_LEVEL := ../..

TOOLNAME = lfort
TOOLALIAS = lfort77

# We don't currently expect production LFort builds to be interested in
# plugins. This is important for startup performance.
ifdef LFORT_IS_PRODUCTION
TOOL_NO_EXPORTS := 1
endif

ifdef LFORT_ORDER_FILE
TOOL_ORDER_FILE := $(LFORT_ORDER_FILE)
endif

# Include tool version information on OS X.
TOOL_INFO_PLIST := Info.plist

# Include this here so we can get the configuration of the targets that have
# been configured for construction. We have to do this early so we can set up
# LINK_COMPONENTS before including Makefile.rules
include $(LFORT_LEVEL)/../../Makefile.config

LINK_COMPONENTS := $(TARGETS_TO_BUILD) asmparser bitreader bitwriter codegen \
                   instrumentation ipo linker selectiondag
USEDLIBS = lfortFrontendTool.a lfortFrontend.a lfortDriver.a \
           lfortSerialization.a lfortCodeGen.a lfortParse.a lfortSema.a

ifeq ($(ENABLE_LFORT_STATIC_ANALYZER),1)
USEDLIBS += lfortStaticAnalyzerFrontend.a lfortStaticAnalyzerCheckers.a \
            lfortStaticAnalyzerCore.a
endif

ifeq ($(ENABLE_LFORT_ARCMT),1)
USEDLIBS += lfortARCMigrate.a
endif

ifeq ($(ENABLE_LFORT_REWRITER),1)
USEDLIBS += lfortRewriteFrontend.a lfortRewriteCore.a
endif

USEDLIBS += lfortAnalysis.a lfortEdit.a lfortAST.a lfortBasic.a lfortLex.a

include $(LFORT_LEVEL)/Makefile

# Set the tool version information values.
ifeq ($(HOST_OS),Darwin)
ifdef LFORT_VENDOR
TOOL_INFO_NAME := $(LFORT_VENDOR) lfort
else
TOOL_INFO_NAME := lfort
endif

ifdef LFORT_VENDOR_UTI
TOOL_INFO_UTI := $(LFORT_VENDOR_UTI)
else
TOOL_INFO_UTI := org.llvm.lfort
endif

TOOL_INFO_VERSION := $(word 3,$(shell grep "LFORT_VERSION " \
	$(PROJ_OBJ_DIR)/$(LFORT_LEVEL)/include/lfort/Basic/Version.inc))
ifdef LLVM_SUBMIT_VERSION
TOOL_INFO_BUILD_VERSION := $(LLVM_SUBMIT_VERSION).$(LLVM_SUBMIT_SUBVERSION)
else
TOOL_INFO_BUILD_VERSION := 
endif
endif
