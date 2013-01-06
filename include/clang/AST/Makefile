LFORT_LEVEL := ../../..
TD_SRC_DIR = $(PROJ_SRC_DIR)/../Basic
BUILT_SOURCES = Attrs.inc AttrImpl.inc StmtNodes.inc DeclNodes.inc \
                CommentNodes.inc CommentHTMLTags.inc \
                CommentHTMLTagsProperties.inc CommentCommandInfo.inc

TABLEGEN_INC_FILES_COMMON = 1

include $(LFORT_LEVEL)/Makefile

$(ObjDir)/Attrs.inc.tmp : $(TD_SRC_DIR)/Attr.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort attribute classes with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-attr-classes -o $(call SYSPATH, $@) \
		-I $(PROJ_SRC_DIR)/../../ $<

$(ObjDir)/AttrImpl.inc.tmp : $(TD_SRC_DIR)/Attr.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort attribute implementations with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-attr-impl -o $(call SYSPATH, $@) \
		-I $(PROJ_SRC_DIR)/../../ $<

$(ObjDir)/StmtNodes.inc.tmp : $(TD_SRC_DIR)/StmtNodes.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort statement node tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-stmt-nodes -o $(call SYSPATH, $@) $<

$(ObjDir)/DeclNodes.inc.tmp : $(TD_SRC_DIR)/DeclNodes.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort declaration node tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-decl-nodes -o $(call SYSPATH, $@) $<

$(ObjDir)/CommentNodes.inc.tmp : $(TD_SRC_DIR)/CommentNodes.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort comment node tables with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-comment-nodes -o $(call SYSPATH, $@) $<

$(ObjDir)/CommentHTMLTags.inc.tmp : $(PROJ_SRC_DIR)/CommentHTMLTags.td $(LFORT_TBLGEN) \
                              $(ObjDir)/.dir
	$(Echo) "Building LFort comment HTML tag matchers with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-comment-html-tags -o $(call SYSPATH, $@) $<

$(ObjDir)/CommentHTMLTagsProperties.inc.tmp : $(PROJ_SRC_DIR)/CommentHTMLTags.td \
                                              $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort comment HTML tag properties with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-comment-html-tags-properties -o $(call SYSPATH, $@) $<

$(ObjDir)/CommentCommandInfo.inc.tmp : $(PROJ_SRC_DIR)/CommentCommands.td \
                                              $(LFORT_TBLGEN) $(ObjDir)/.dir
	$(Echo) "Building LFort comment command info with tblgen"
	$(Verb) $(LFortTableGen) -gen-lfort-comment-command-info -o $(call SYSPATH, $@) $<

