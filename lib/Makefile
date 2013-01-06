##===- lib/Makefile ----------------------------------------*- Makefile -*-===##
# 
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##
LFORT_LEVEL := ..

# ARCMigrate and Rewrite are always needed because of liblfort.
PARALLEL_DIRS = Headers Basic Lex Parse AST Sema CodeGen Analysis Frontend \
                FrontendTool Tooling Driver Format Edit Rewrite \
                Serialization

include $(LFORT_LEVEL)/../../Makefile.config

ifeq ($(ENABLE_LFORT_REWRITER),1)
PARALLEL_DIRS += ASTMatchers
endif

ifeq ($(ENABLE_LFORT_STATIC_ANALYZER),1)
PARALLEL_DIRS += StaticAnalyzer
endif

include $(LFORT_LEVEL)/Makefile
