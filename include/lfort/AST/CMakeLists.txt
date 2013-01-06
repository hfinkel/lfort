lfort_tablegen(Attrs.inc -gen-lfort-attr-classes
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrClasses)

lfort_tablegen(AttrImpl.inc -gen-lfort-attr-impl
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrImpl)

lfort_tablegen(StmtNodes.inc -gen-lfort-stmt-nodes
  SOURCE ../Basic/StmtNodes.td
  TARGET LFortStmtNodes)

lfort_tablegen(DeclNodes.inc -gen-lfort-decl-nodes
  SOURCE ../Basic/DeclNodes.td
  TARGET LFortDeclNodes)

lfort_tablegen(CommentNodes.inc -gen-lfort-comment-nodes
  SOURCE ../Basic/CommentNodes.td
  TARGET LFortCommentNodes)

lfort_tablegen(CommentHTMLTags.inc -gen-lfort-comment-html-tags
  SOURCE CommentHTMLTags.td
  TARGET LFortCommentHTMLTags)

lfort_tablegen(CommentHTMLTagsProperties.inc -gen-lfort-comment-html-tags-properties
  SOURCE CommentHTMLTags.td
  TARGET LFortCommentHTMLTagsProperties)

lfort_tablegen(CommentCommandInfo.inc -gen-lfort-comment-command-info
  SOURCE CommentCommands.td
  TARGET LFortCommentCommandInfo)

