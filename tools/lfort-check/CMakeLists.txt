set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

add_lfort_executable(lfort-check
  LFortCheck.cpp
  )

target_link_libraries(lfort-check
  lfortTooling
  lfortBasic
  lfortRewriteFrontend
  )

install(TARGETS lfort-check
  RUNTIME DESTINATION bin)
