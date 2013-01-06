lfort_tablegen(Options.inc -gen-opt-parser-defs
  SOURCE Options.td
  TARGET LFortDriverOptions)

lfort_tablegen(CC1AsOptions.inc -gen-opt-parser-defs
  SOURCE CC1AsOptions.td
  TARGET LFortCC1AsOptions)
