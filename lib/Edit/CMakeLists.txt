add_lfort_library(lfortEdit
  Commit.cpp
  EditedSource.cpp
  RewriteObjCFoundationAPI.cpp
  )

add_dependencies(lfortEdit
  LFortAttrClasses
  LFortAttrList
  LFortCommentNodes
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortStmtNodes
  )

target_link_libraries(lfortEdit
  lfortBasic
  lfortAST
  lfortLex
  )
