lfort_tablegen(AttrTemplateInstantiate.inc -gen-lfort-attr-template-instantiate
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrTemplateInstantiate)
  
lfort_tablegen(AttrParsedAttrList.inc -gen-lfort-attr-parsed-attr-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrParsedAttrList)

lfort_tablegen(AttrParsedAttrKinds.inc -gen-lfort-attr-parsed-attr-kinds
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE ../Basic/Attr.td
  TARGET LFortAttrParsedAttrKinds)