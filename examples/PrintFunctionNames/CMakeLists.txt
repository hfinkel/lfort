set(MODULE TRUE)

set( LLVM_LINK_COMPONENTS support mc)

add_lfort_library(PrintFunctionNames PrintFunctionNames.cpp)

add_dependencies(PrintFunctionNames
  LFortAttrClasses
  LFortAttrList
  LFortCommentNodes
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortStmtNodes
  )

target_link_libraries(PrintFunctionNames
  lfortFrontend
  lfortAST
  )

set_target_properties(PrintFunctionNames
  PROPERTIES
  LINKER_LANGUAGE CXX
  PREFIX "")
