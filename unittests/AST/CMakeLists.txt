add_lfort_unittest(ASTTests
  CommentLexer.cpp
  CommentParser.cpp
  DeclPrinterTest.cpp
  SourceLocationTest.cpp
  StmtPrinterTest.cpp
  )

target_link_libraries(ASTTests
  lfortAST lfortASTMatchers lfortTooling
  )
