set(LLVM_NO_RTTI 1)

include_directories( ${CMAKE_CURRENT_BINARY_DIR}/../Checkers )

add_lfort_library(lfortStaticAnalyzerFrontend
  AnalysisConsumer.cpp
  CheckerRegistration.cpp
  FrontendActions.cpp
  )

add_dependencies(lfortStaticAnalyzerFrontend
  lfortStaticAnalyzerCheckers
  lfortStaticAnalyzerCore
  LFortAttrClasses
  LFortAttrList
  LFortCommentNodes
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortDiagnosticFrontend
  LFortStmtNodes
  )

target_link_libraries(lfortStaticAnalyzerFrontend
  lfortBasic
  lfortLex
  lfortAST
  lfortFrontend
  lfortRewriteCore
  lfortRewriteFrontend
  lfortStaticAnalyzerCheckers
  )
