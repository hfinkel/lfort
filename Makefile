##===- Makefile --------------------------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
#
##===----------------------------------------------------------------------===##

# If LFORT_LEVEL is not set, then we are the top-level Makefile. Otherwise, we
# are being included from a subdirectory makefile.

ifndef LFORT_LEVEL

IS_TOP_LEVEL := 1
LFORT_LEVEL := .
DIRS := utils/TableGen include lib tools runtime docs unittests

PARALLEL_DIRS :=

ifeq ($(BUILD_EXAMPLES),1)
  PARALLEL_DIRS += examples
endif
endif

ifeq ($(MAKECMDGOALS),libs-only)
  DIRS := $(filter-out tools docs, $(DIRS))
  OPTIONAL_DIRS :=
endif
ifeq ($(BUILD_LFORT_ONLY),YES)
  DIRS := $(filter-out docs unittests, $(DIRS))
  OPTIONAL_DIRS :=
endif

###
# Common Makefile code, shared by all LFort Makefiles.

# Set LLVM source root level.
LEVEL := $(LFORT_LEVEL)/../..

# Include LLVM common makefile.
include $(LEVEL)/Makefile.common

ifneq ($(ENABLE_DOCS),1)
  DIRS := $(filter-out docs, $(DIRS))
endif

# Set common LFort build flags.
CPP.Flags += -I$(PROJ_SRC_DIR)/$(LFORT_LEVEL)/include -I$(PROJ_OBJ_DIR)/$(LFORT_LEVEL)/include
ifdef LFORT_VENDOR
CPP.Flags += -DLFORT_VENDOR='"$(LFORT_VENDOR) "'
endif
ifdef LFORT_REPOSITORY_STRING
CPP.Flags += -DLFORT_REPOSITORY_STRING='"$(LFORT_REPOSITORY_STRING)"'
endif

# Disable -fstrict-aliasing. Darwin disables it by default (and LLVM doesn't
# work with it enabled with GCC), LFort/llvm-gcc don't support it yet, and newer
# GCC's have false positive warnings with it on Linux (which prove a pain to
# fix). For example:
#   http://gcc.gnu.org/PR41874
#   http://gcc.gnu.org/PR41838
#
# We can revisit this when LLVM/LFort support it.
CXX.Flags += -fno-strict-aliasing

# Set up LFort's tblgen.
ifndef LFORT_TBLGEN
  ifeq ($(LLVM_CROSS_COMPILING),1)
    LFORT_TBLGEN := $(BuildLLVMToolDir)/lfort-tblgen$(BUILD_EXEEXT)
  else
    LFORT_TBLGEN := $(LLVMToolDir)/lfort-tblgen$(EXEEXT)
  endif
endif
LFortTableGen = $(LFORT_TBLGEN) $(TableGen.Flags)

###
# LFort Top Level specific stuff.

ifeq ($(IS_TOP_LEVEL),1)

ifneq ($(PROJ_SRC_ROOT),$(PROJ_OBJ_ROOT))
$(RecursiveTargets)::
	$(Verb) for dir in test unittests; do \
	  if [ -f $(PROJ_SRC_DIR)/$${dir}/Makefile ] && [ ! -f $${dir}/Makefile ]; then \
	    $(MKDIR) $${dir}; \
	    $(CP) $(PROJ_SRC_DIR)/$${dir}/Makefile $${dir}/Makefile; \
	  fi \
	done
endif

test::
	@ $(MAKE) -C test

report::
	@ $(MAKE) -C test report

clean::
	@ $(MAKE) -C test clean

libs-only: all

tags::
	$(Verb) etags `find . -type f -name '*.h' -or -name '*.cpp' | \
	  grep -v /lib/Headers | grep -v /test/`

cscope.files:
	find tools lib include -name '*.cpp' \
	                    -or -name '*.def' \
	                    -or -name '*.td' \
	                    -or -name '*.h' > cscope.files

.PHONY: test report clean cscope.files

endif
