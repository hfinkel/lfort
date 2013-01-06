# TODO: Add -maltivec when ARCH is PowerPC.

set(LLVM_LINK_COMPONENTS support)

add_lfort_library(lfortLex
  HeaderMap.cpp
  HeaderSearch.cpp
  Lexer.cpp
  LiteralSupport.cpp
  MacroArgs.cpp
  MacroInfo.cpp
  ModuleMap.cpp
  PPCaching.cpp
  PPCallbacks.cpp
  PPConditionalDirectiveRecord.cpp
  PPDirectives.cpp
  PPExpressions.cpp
  PPLexerChange.cpp
  PPMacroExpansion.cpp
  PTHLexer.cpp
  Pragma.cpp
  PreprocessingRecord.cpp
  Preprocessor.cpp
  PreprocessorLexer.cpp
  ScratchBuffer.cpp
  TokenConcatenation.cpp
  TokenLexer.cpp
  )

add_dependencies(lfortLex
  LFortAttrSpellings
  LFortDiagnosticCommon
  LFortDiagnosticLex
  )

target_link_libraries(lfortLex
  lfortBasic
  )
