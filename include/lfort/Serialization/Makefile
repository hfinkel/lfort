LFORT_LEVEL := ../../..
TD_SRC_DIR = $(PROJ_SRC_DIR)/../Basic
BUILT_SOURCES = AttrPCHRead.inc AttrPCHWrite.inc

TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

$(ObjDir)/AttrPCHRead.inc.tmp : $(TD_SRC_DIR)/Attr.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort PCH reader with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-attr-pch-read -o $(call SYSPATH, $@) \
		-I $(PROJ_SRC_DIR)/../../ $<

$(ObjDir)/AttrPCHWrite.inc.tmp : $(TD_SRC_DIR)/Attr.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort PCH writer with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-attr-pch-write -o $(call SYSPATH, $@) \
		-I $(PROJ_SRC_DIR)/../../ $<
