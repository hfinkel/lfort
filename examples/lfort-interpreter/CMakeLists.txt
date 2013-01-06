set(LLVM_LINK_COMPONENTS
  jit
  interpreter
  nativecodegen
  asmparser
  bitreader
  bitwriter
  codegen
  ipo
  linker
  selectiondag
  )

add_lfort_executable(lfort-interpreter
  main.cpp
  )

add_dependencies(lfort-interpreter
  lfort-headers
  )

target_link_libraries(lfort-interpreter
  lfortFrontend
  lfortSerialization
  lfortDriver
  lfortCodeGen
  lfortSema
  lfortStaticAnalyzerFrontend
  lfortStaticAnalyzerCheckers
  lfortStaticAnalyzerCore
  lfortAnalysis
  lfortRewriteCore
  lfortRewriteFrontend
  lfortAST
  lfortParse
  lfortLex
  lfortBasic
  )
