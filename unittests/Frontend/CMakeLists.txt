set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

add_lfort_unittest(FrontendTests
  FrontendActionTest.cpp
  )
target_link_libraries(FrontendTests
  lfortFrontend
  )
