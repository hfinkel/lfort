add_lfort_library(lfortDriver
  Action.cpp
  Arg.cpp
  ArgList.cpp
  CC1AsOptions.cpp
  Compilation.cpp
  Driver.cpp
  DriverOptions.cpp
  Job.cpp
  Option.cpp
  OptTable.cpp
  Phases.cpp
  Tool.cpp
  ToolChain.cpp
  ToolChains.cpp
  WindowsToolChain.cpp
  Tools.cpp
  Types.cpp
  )

add_dependencies(lfortDriver
  LFortAttrList
  LFortCC1AsOptions
  LFortDiagnosticCommon
  LFortDiagnosticDriver
  LFortDriverOptions
  )

target_link_libraries(lfortDriver
  lfortBasic
  )
