add_subdirectory(liblfort)
add_subdirectory(c-index-test)
add_subdirectory(arcmt-test)
add_subdirectory(c-arcmt-test)
add_subdirectory(diagtool)
add_subdirectory(driver)
add_subdirectory(lfort-check)

# We support checking out the lfort-tools-extra repository into the 'extra'
# subdirectory. It contains tools developed as part of the LFort/LLVM project
# on top of the LFort tooling platform. We keep them in a separate repository
# to keep the primary LFort repository small and focused.
# It also may be included by LLVM_EXTERNAL_LFORT_TOOLS_EXTRA_SOURCE_DIR.
add_llvm_external_project(lfort-tools-extra extra)
