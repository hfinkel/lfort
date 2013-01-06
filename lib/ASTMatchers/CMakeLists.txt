set(LLVM_LINK_COMPONENTS support)

add_lfort_library(lfortASTMatchers
  ASTMatchFinder.cpp
  ASTMatchersInternal.cpp
  )

add_dependencies(lfortASTMatchers
  LFortAttrClasses
  LFortAttrImpl
  LFortAttrList
  LFortCommentNodes
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortStmtNodes
  )

target_link_libraries(lfortASTMatchers
  lfortBasic
  lfortAST
  )
