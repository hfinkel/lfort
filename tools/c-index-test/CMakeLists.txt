set( LLVM_LINK_COMPONENTS
  support
  mc
  )

add_lfort_executable(c-index-test
  c-index-test.c
  )

if(NOT MSVC)
  set_property(
    SOURCE c-index-test.c
    PROPERTY COMPILE_FLAGS "-std=c89"
    )
endif()

target_link_libraries(c-index-test
  liblfort
  )

set_target_properties(c-index-test
  PROPERTIES
  LINKER_LANGUAGE CXX)

# If libxml2 is available, make it available for c-index-test.
if (LFORT_HAVE_LIBXML)
  include_directories(${LIBXML2_INCLUDE_DIR})
  target_link_libraries(c-index-test ${LIBXML2_LIBRARIES})
endif()
