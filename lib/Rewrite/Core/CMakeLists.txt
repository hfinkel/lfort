add_lfort_library(lfortRewriteCore
  DeltaTree.cpp
  HTMLRewrite.cpp
  RewriteRope.cpp
  Rewriter.cpp
  TokenRewriter.cpp
  )

add_dependencies(lfortRewriteCore
  LFortAttrClasses
  LFortAttrList
  LFortAttrParsedAttrList
  LFortCommentNodes
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortDiagnosticFrontend
  LFortStmtNodes
  )

target_link_libraries(lfortRewriteCore
  lfortBasic
  lfortAST
  lfortParse
  )
