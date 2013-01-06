lfort_tablegen(AttrLateParsed.inc -gen-lfort-attr-late-parsed-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrLateParsed)
