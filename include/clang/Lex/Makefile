LFORT_LEVEL := ../../..
TD_SRC_DIR = $(PROJ_SRC_DIR)/../Basic
BUILT_SOURCES = AttrSpellings.inc 

TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

$(ObjDir)/AttrSpellings.inc.tmp : $(TD_SRC_DIR)/Attr.td $(LFORT_TBLGEN) \
                                  $(ObjDir)/.dir
	$(Echo) "Building LFort attribute spellings with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-attr-spelling-list -o $(call SYSPATH, $@) \
		-I $(PROJ_SRC_DIR)/../../ $<
