set(LLVM_LINK_COMPONENTS mc)

add_lfort_library(lfortBasic
  Builtins.cpp
  ConvertUTF.c
  ConvertUTFWrapper.cpp
  Diagnostic.cpp
  DiagnosticIDs.cpp
  FileManager.cpp
  FileSystemStatCache.cpp
  IdentifierTable.cpp
  LangOptions.cpp
  Module.cpp
  ObjCRuntime.cpp
  OperatorPrecedence.cpp
  SourceLocation.cpp
  SourceManager.cpp
  TargetInfo.cpp
  Targets.cpp
  TokenKinds.cpp
  Version.cpp
  VersionTuple.cpp
  )

# Determine Subversion revision.
# FIXME: This only gets updated when CMake is run, so this revision number
# may be out-of-date!
if( NOT IS_SYMLINK "${LFORT_SOURCE_DIR}" )  # See PR 8437
  find_package(Subversion)
endif()
if (Subversion_FOUND AND EXISTS "${LFORT_SOURCE_DIR}/.svn")
  Subversion_WC_INFO(${LFORT_SOURCE_DIR} LFORT)
  set_source_files_properties(Version.cpp
    PROPERTIES COMPILE_DEFINITIONS "SVN_REVISION=\"${LFORT_WC_REVISION}\"")
endif()

add_dependencies(lfortBasic
  LFortARMNeon
  LFortAttrList
  LFortDiagnosticAnalysis
  LFortDiagnosticAST
  LFortDiagnosticComment
  LFortDiagnosticCommon
  LFortDiagnosticDriver
  LFortDiagnosticFrontend
  LFortDiagnosticGroups
  LFortDiagnosticIndexName
  LFortDiagnosticLex
  LFortDiagnosticParse
  LFortDiagnosticSema
  LFortDiagnosticSerialization
  )
