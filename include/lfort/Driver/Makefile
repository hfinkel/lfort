LFORT_LEVEL := ../../..
BUILT_SOURCES = Options.inc CC1AsOptions.inc

TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

$(ObjDir)/Options.inc.tmp : Options.td CC1Options.td OptParser.td $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort Driver Option tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-opt-parser-defs -o $(call SYSPATH, $@) $<

$(ObjDir)/CC1AsOptions.inc.tmp : CC1AsOptions.td OptParser.td $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort CC1 Assembler Option tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-opt-parser-defs -o $(call SYSPATH, $@) $<
