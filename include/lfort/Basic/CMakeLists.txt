macro(lfort_diag_gen component)
  lfort_tablegen(Diagnostic${component}Kinds.inc
    -gen-lfort-diags-defs -lfort-component=${component}
    SOURCE Diagnostic.td
    TARGET LFortDiagnostic${component})
endmacro(lfort_diag_gen)

lfort_diag_gen(Analysis)
lfort_diag_gen(AST)
lfort_diag_gen(Comment)
lfort_diag_gen(Common)
lfort_diag_gen(Driver)
lfort_diag_gen(Frontend)
lfort_diag_gen(Lex)
lfort_diag_gen(Parse)
lfort_diag_gen(Sema)
lfort_diag_gen(Serialization)
lfort_tablegen(DiagnosticGroups.inc -gen-lfort-diag-groups
  SOURCE Diagnostic.td
  TARGET LFortDiagnosticGroups)

lfort_tablegen(DiagnosticIndexName.inc -gen-lfort-diags-index-name
  SOURCE Diagnostic.td
  TARGET LFortDiagnosticIndexName)

lfort_tablegen(AttrList.inc -gen-lfort-attr-list
  -I ${CMAKE_CURRENT_SOURCE_DIR}/../../
  SOURCE Attr.td
  TARGET LFortAttrList)

# ARM NEON
lfort_tablegen(arm_neon.inc -gen-arm-neon-sema
  SOURCE arm_neon.td
  TARGET LFortARMNeon)
