add_lfort_unittest(LexTests
  LexerTest.cpp
  PPCallbacksTest.cpp
  PPConditionalDirectiveRecordTest.cpp
  )

target_link_libraries(LexTests
  lfortLex
  )
