# Test runner infrastructure for LFort. This configures the LFort test trees
# for use by Lit, and delegates to LLVM's lit test handlers.
#
# If this is a stand-alone LFort build, we fake up our own Lit support here
# rather than relying on LLVM's.

set(LFORT_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/..")
set(LFORT_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/..")

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
  )

configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/Unit/lit.site.cfg.in
  ${CMAKE_CURRENT_BINARY_DIR}/Unit/lit.site.cfg
  )

if( PATH_TO_LLVM_BUILD )
  set(LFORT_TEST_EXTRA_ARGS "--path=${LFORT_BINARY_DIR}/bin/${CMAKE_CFG_INTDIR}")
endif()

option(LFORT_TEST_USE_VG "Run LFort tests under Valgrind" OFF)
if(LFORT_TEST_USE_VG)
  set(LFORT_TEST_EXTRA_ARGS ${LFORT_TEST_EXTRA_ARGS} "--vg")
endif ()

if( NOT LFORT_BUILT_STANDALONE )

  set(LFORT_TEST_DEPS
    lfort lfort-headers
    c-index-test diagtool arcmt-test c-arcmt-test
    lfort-check
    llvm-dis llc opt FileCheck count not
    )
  set(LFORT_TEST_PARAMS
    lfort_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
    )

  if(LLVM_INCLUDE_TESTS)
    list(APPEND LFORT_TEST_DEPS LFortUnitTests)
    list(APPEND LFORT_TEST_PARAMS
      lfort_unit_site_config=${CMAKE_CURRENT_BINARY_DIR}/Unit/lit.site.cfg
      )
  endif()
  add_lit_testsuite(check-lfort "Running the LFort regression tests"
    ${CMAKE_CURRENT_BINARY_DIR}
    PARAMS ${LFORT_TEST_PARAMS}
    DEPENDS ${LFORT_TEST_DEPS}
    ARGS ${LFORT_TEST_EXTRA_ARGS}
    )
  set_target_properties(check-lfort PROPERTIES FOLDER "LFort tests")

else()

  include(FindPythonInterp)
  if(PYTHONINTERP_FOUND)
    if( LLVM_MAIN_SRC_DIR )
      set(LIT "${LLVM_MAIN_SRC_DIR}/utils/lit/lit.py")
    else()
      set(LIT "${PATH_TO_LLVM_BUILD}/bin/${CMAKE_CFG_INTDIR}/llvm-lit")
      # Installed LLVM does not contain ${CMAKE_CFG_INTDIR} in paths.
      if( NOT EXISTS ${LIT} )
        set(LIT "${PATH_TO_LLVM_BUILD}/bin/llvm-lit")
      endif()
    endif()

    set(LIT_ARGS "${LFORT_TEST_EXTRA_ARGS} ${LLVM_LIT_ARGS}")
    separate_arguments(LIT_ARGS)

    add_custom_target(check-lfort
      COMMAND ${PYTHON_EXECUTABLE}
              ${LIT}
              --param lfort_site_config=${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg
              --param build_config=${CMAKE_CFG_INTDIR}
              --param build_mode=${RUNTIME_BUILD_MODE}
              ${LIT_ARGS}
              ${CMAKE_CURRENT_BINARY_DIR}
      COMMENT "Running LFort regression tests"
      DEPENDS lfort lfort-headers
              c-index-test diagtool arcmt-test c-arcmt-test
              lfort-check
      )
    set_target_properties(check-lfort PROPERTIES FOLDER "LFort tests")
  endif()

endif()

# Add a legacy target spelling: lfort-test
add_custom_target(lfort-test)
add_dependencies(lfort-test check-lfort)
set_target_properties(lfort-test PROPERTIES FOLDER "LFort tests")
