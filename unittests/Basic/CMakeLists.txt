add_lfort_unittest(BasicTests
  FileManagerTest.cpp
  SourceManagerTest.cpp
  )

target_link_libraries(BasicTests
  lfortBasic
  lfortLex
  )
