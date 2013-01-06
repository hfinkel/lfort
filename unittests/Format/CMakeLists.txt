set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

add_lfort_unittest(FormatTests
  FormatTest.cpp
  )

target_link_libraries(FormatTests
  lfortAST
  lfortFormat
  lfortTooling
  lfortRewriteCore
  )
