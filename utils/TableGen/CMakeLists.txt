set(LLVM_LINK_COMPONENTS Support)

add_tablegen(lfort-tblgen LFORT
  LFortASTNodesEmitter.cpp
  LFortAttrEmitter.cpp
  LFortCommentCommandInfoEmitter.cpp
  LFortCommentHTMLTagsEmitter.cpp
  LFortDiagnosticsEmitter.cpp
  LFortSACheckersEmitter.cpp
  NeonEmitter.cpp
  OptParserEmitter.cpp
  TableGen.cpp
  )
