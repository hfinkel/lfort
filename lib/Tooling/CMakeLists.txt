set(LLVM_LINK_COMPONENTS support)

add_lfort_library(lfortTooling
  ArgumentsAdjusters.cpp
  CommonOptionsParser.cpp
  CompilationDatabase.cpp
  FileMatchTrie.cpp
  JSONCompilationDatabase.cpp
  Refactoring.cpp
  RefactoringCallbacks.cpp
  Tooling.cpp
  )

add_dependencies(lfortTooling
  LFortAttrClasses
  LFortAttrList
  LFortDeclNodes
  LFortDiagnosticCommon
  LFortDiagnosticFrontend
  LFortStmtNodes
  )

target_link_libraries(lfortTooling
  lfortBasic
  lfortFrontend
  lfortAST
  lfortASTMatchers
  lfortRewriteCore
  lfortRewriteFrontend
  )
