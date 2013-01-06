add_custom_target(LFortUnitTests)
set_target_properties(LFortUnitTests PROPERTIES FOLDER "LFort tests")

# add_lfort_unittest(test_dirname file1.cpp file2.cpp)
#
# Will compile the list of files together and link against the lfort
# Produces a binary named 'basename(test_dirname)'.
function(add_lfort_unittest test_dirname)
  add_unittest(LFortUnitTests ${test_dirname} ${ARGN})
endfunction()

add_subdirectory(ASTMatchers)
add_subdirectory(AST)
add_subdirectory(Basic)
add_subdirectory(Lex)
add_subdirectory(Frontend)
add_subdirectory(Tooling)
add_subdirectory(Format)
