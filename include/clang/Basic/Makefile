LFORT_LEVEL := ../../..
BUILT_SOURCES = \
	DiagnosticAnalysisKinds.inc DiagnosticASTKinds.inc \
	DiagnosticCommentKinds.inc \
	DiagnosticCommonKinds.inc DiagnosticDriverKinds.inc \
	DiagnosticFrontendKinds.inc DiagnosticLexKinds.inc \
	DiagnosticParseKinds.inc DiagnosticSemaKinds.inc \
	DiagnosticSerializationKinds.inc \
	DiagnosticIndexName.inc DiagnosticGroups.inc AttrList.inc arm_neon.inc \
	Version.inc

TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

INPUT_TDS = $(wildcard $(PROJ_SRC_DIR)/Diagnostic*.td)

# Compute the LFort version from the LLVM version, unless specified explicitly.
ifndef LFORT_VERSION
LFORT_VERSION := $(subst svn,,$(LLVMVersion))
LFORT_VERSION := $(subst rc,,$(LFORT_VERSION))
endif

LFORT_VERSION_COMPONENTS := $(subst ., ,$(LFORT_VERSION))
LFORT_VERSION_MAJOR := $(word 1,$(LFORT_VERSION_COMPONENTS))
LFORT_VERSION_MINOR := $(word 2,$(LFORT_VERSION_COMPONENTS))
LFORT_VERSION_PATCHLEVEL := $(word 3,$(LFORT_VERSION_COMPONENTS))
ifeq ($(LFORT_VERSION_PATCHLEVEL),)
LFORT_HAS_VERSION_PATCHLEVEL := 0
else
LFORT_HAS_VERSION_PATCHLEVEL := 1
endif

$(ObjDir)/Diagnostic%Kinds.inc.tmp : Diagnostic.td $(INPUT_TDS) $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort $(patsubst Diagnostic%Kinds.inc.tmp,%,$(@F)) diagnostic tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-diags-defs -lfort-component=$(patsubst Diagnostic%Kinds.inc.tmp,%,$(@F)) -o $(call SYSPATH, $@) $<

$(ObjDir)/DiagnosticIndexName.inc.tmp : Diagnostic.td $(INPUT_TDS) $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort diagnostic name index with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-diags-index-name -o $(call SYSPATH, $@) $<

$(ObjDir)/DiagnosticGroups.inc.tmp : Diagnostic.td DiagnosticGroups.td $(INPUT_TDS) $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort diagnostic groups with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-diag-groups -o $(call SYSPATH, $@) $<

$(ObjDir)/AttrList.inc.tmp : Attr.td $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort attribute list with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-attr-list -o $(call SYSPATH, $@) \
	  -I $(PROJ_SRC_DIR)/../.. $<

$(ObjDir)/arm_neon.inc.tmp : arm_neon.td $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort arm_neon.inc with tblgen"
	$(Verb) $(LFortTableGen) -gen-arm-neon-sema -o $(call SYSPATH, $@) $<

$(ObjDir)/Version.inc.tmp : Version.inc.in Makefile $(LLVM_OBJ_ROOT)/Makefile.config $(ObjDir)/.dir
	$(Echo) "Updating LFort version info."
	$(Verb)sed -e "s#@LFORT_VERSION@#$(LFORT_VERSION)#g" \
	           -e "s#@LFORT_VERSION_MAJOR@#$(LFORT_VERSION_MAJOR)#g" \
	           -e "s#@LFORT_VERSION_MINOR@#$(LFORT_VERSION_MINOR)#g" \
	           -e "s#@LFORT_VERSION_PATCHLEVEL@#$(LFORT_VERSION_PATCHLEVEL)#g" \
	           -e "s#@LFORT_HAS_VERSION_PATCHLEVEL@#$(LFORT_HAS_VERSION_PATCHLEVEL)#g" \
	           $< > $@
