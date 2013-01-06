lfort_tablegen(AttrPCHRead.inc -gen-lfort-attr-pch-read
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrPCHRead)

lfort_tablegen(AttrPCHWrite.inc -gen-lfort-attr-pch-write
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrPCHWrite)
