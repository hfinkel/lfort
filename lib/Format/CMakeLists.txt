set(LLVM_LINK_COMPONENTS support)

add_lfort_library(lfortFormat
  UnwrappedLineParser.cpp
  Format.cpp
  )

add_dependencies(lfortFormat
  LFortAttrClasses
  LFortAttrList
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortDiagnosticFrontend
  LFortStmtNodes
  )

target_link_libraries(lfortFormat
  lfortBasic
  lfortFrontend
  lfortAST
  lfortASTMatchers
  lfortRewriteCore
  lfortRewriteFrontend
  lfortTooling
  )
