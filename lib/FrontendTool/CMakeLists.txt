add_lfort_library(lfortFrontendTool
  ExecuteCompilerInvocation.cpp
  )

add_dependencies(lfortFrontendTool
  LFortDiagnosticCommon
  LFortDiagnosticFrontend
  LFortDriverOptions
  )

target_link_libraries(lfortFrontendTool
  lfortDriver
  lfortFrontend
  lfortRewriteCore
  lfortRewriteFrontend
  lfortCodeGen
  lfortStaticAnalyzerFrontend
  lfortStaticAnalyzerCheckers
  lfortStaticAnalyzerCore
  )
