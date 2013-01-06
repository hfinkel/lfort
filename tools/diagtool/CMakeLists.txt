set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

add_lfort_executable(diagtool
  diagtool_main.cpp
  DiagTool.cpp
  DiagnosticNames.cpp
  ListWarnings.cpp
  ShowEnabledWarnings.cpp
  TreeView.cpp
)

add_dependencies(diagtool
  LFortDiagnosticIndexName
  )

target_link_libraries(diagtool
  lfortBasic
  lfortLex
  lfortSema
  lfortFrontend
  )

if(UNIX)
  set(LFORTXX_LINK_OR_COPY create_symlink)
else()
  set(LFORTXX_LINK_OR_COPY copy)
endif()
