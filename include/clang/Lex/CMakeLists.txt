lfort_tablegen(AttrSpellings.inc -gen-lfort-attr-spelling-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrSpellings
  DEPENDS AttrSpellings.inc)
