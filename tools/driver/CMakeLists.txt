set( LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  bitreader
  bitwriter
  codegen
  instrumentation
  ipo
  linker
  selectiondag
  )

add_lfort_executable(lfort
  driver.cpp
  cc1_main.cpp
  cc1as_main.cpp
  )

target_link_libraries(lfort
  lfortFrontendTool
  lfortAST
  lfortAnalysis
  lfortBasic
  lfortCodeGen
  lfortDriver
  lfortEdit
  lfortFrontend
  lfortLex
  lfortParse
  lfortEdit
  lfortARCMigrate
  lfortRewriteCore
  lfortRewriteFrontend
  lfortSema
  lfortSerialization
  lfortStaticAnalyzerFrontend
  lfortStaticAnalyzerCheckers
  lfortStaticAnalyzerCore
  )

set_target_properties(lfort PROPERTIES VERSION ${LFORT_EXECUTABLE_VERSION})

add_dependencies(lfort lfort-headers)

if(UNIX)
  set(LFORTXX_LINK_OR_COPY create_symlink)
# Create a relative symlink
  set(lfort_binary "lfort${CMAKE_EXECUTABLE_SUFFIX}")
else()
  set(LFORTXX_LINK_OR_COPY copy)
  set(lfort_binary "${LLVM_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/lfort${CMAKE_EXECUTABLE_SUFFIX}")
endif()

# Create the lfort++ symlink in the build directory.
set(lfort_pp "${LLVM_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}/lfort++${CMAKE_EXECUTABLE_SUFFIX}")
add_custom_command(TARGET lfort POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E ${LFORTXX_LINK_OR_COPY} "${lfort_binary}" "${lfort_pp}")

set_property(DIRECTORY APPEND
  PROPERTY ADDITIONAL_MAKE_CLEAN_FILES ${lfort_pp})

install(TARGETS lfort
  RUNTIME DESTINATION bin)

# Create the lfort++ symlink at installation time.
install(SCRIPT lfort_symlink.cmake -DCMAKE_INSTALL_PREFIX=\"${CMAKE_INSTALL_PREFIX}\")
