set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

add_lfort_executable(arcmt-test
  arcmt-test.cpp
  )

target_link_libraries(arcmt-test
  lfortARCMigrate
  lfortEdit
  lfortRewriteCore
  )
