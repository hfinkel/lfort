##===- lfort/lib/Headers/Makefile --------------------------*- Makefile -*-===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is distributed under the University of Illinois Open Source
# License. See LICENSE.TXT for details.
# 
##===----------------------------------------------------------------------===##

LFORT_LEVEL := ../..

BUILT_SOURCES = arm_neon.h.inc
TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

LFORT_VERSION := $(word 3,$(shell grep "LFORT_VERSION " \
	$(PROJ_OBJ_DIR)/$(LFORT_LEVEL)/include/lfort/Basic/Version.inc))

HeaderDir := $(PROJ_OBJ_ROOT)/$(BuildMode)/lib/lfort/$(LFORT_VERSION)/include

HEADERS := $(notdir $(wildcard $(PROJ_SRC_DIR)/*.h))

OBJHEADERS := $(addprefix $(HeaderDir)/, $(HEADERS))


$(OBJHEADERS): $(HeaderDir)/%.h: $(PROJ_SRC_DIR)/%.h $(HeaderDir)/.dir $(HeaderDir)/arm_neon.h
	$(Verb) cp $< $@
	$(Echo) Copying $(notdir $<) to build dir

$(HeaderDir)/arm_neon.h: $(BUILT_SOURCES) $(HeaderDir)/.dir
	$(Verb) cp $< $@
	$(Echo) Copying $(notdir $<) to build dir

$(HeaderDir)/module.map: $(PROJ_SRC_DIR)/module.map $(HeaderDir)/.dir
	$(Verb) cp $< $@
	$(Echo) Copying $(notdir $<) to build dir


# Hook into the standard Makefile rules.
all-local:: $(OBJHEADERS) $(HeaderDir)/module.map

PROJ_headers := $(DESTDIR)$(PROJ_prefix)/lib/lfort/$(LFORT_VERSION)/include

INSTHEADERS := $(addprefix $(PROJ_headers)/, $(HEADERS))
INSTHEADERS += $(PROJ_headers)/arm_neon.h

$(PROJ_headers):
	$(Verb) $(MKDIR) $@

$(INSTHEADERS): $(PROJ_headers)/%.h: $(HeaderDir)/%.h | $(PROJ_headers)
	$(Verb) $(DataInstall) $< $(PROJ_headers)
	$(Echo) Installing compiler include file: $(notdir $<)

$(PROJ_headers)/module.map: $(HeaderDir)/module.map | $(PROJ_headers)
	$(Verb) $(DataInstall) $< $(PROJ_headers)
	$(Echo) Installing compiler module map file: $(notdir $<)


install-local:: $(INSTHEADERS) $(PROJ_headers)/module.map

$(ObjDir)/arm_neon.h.inc.tmp : $(LFORT_LEVEL)/include/lfort/Basic/arm_neon.td $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort arm_neon.h.inc with tblgen"
	$(Verb) $(LFortTableGen) -gen-arm-neon -o $(call SYSPATH, $@) $<
