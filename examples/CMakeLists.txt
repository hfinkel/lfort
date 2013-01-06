if(NOT LFORT_BUILD_EXAMPLES)
  set_property(DIRECTORY PROPERTY EXCLUDE_FROM_ALL ON)
endif()

add_subdirectory(analyzer-plugin)
add_subdirectory(lfort-interpreter)
add_subdirectory(PrintFunctionNames)
