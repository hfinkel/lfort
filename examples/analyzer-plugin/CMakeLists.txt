set(MODULE TRUE)

set( LLVM_LINK_COMPONENTS support mc)

add_lfort_library(SampleAnalyzerPlugin MainCallChecker.cpp)

add_dependencies(SampleAnalyzerPlugin
  LFortAttrClasses
  LFortAttrList
  LFortCommentNodes
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortStmtNodes
  )

target_link_libraries(SampleAnalyzerPlugin
  lfortStaticAnalyzerCore
  )

set_target_properties(SampleAnalyzerPlugin
  PROPERTIES
  LINKER_LANGUAGE CXX
  PREFIX "")
