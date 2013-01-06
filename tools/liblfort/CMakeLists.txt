set(LLVM_LINK_COMPONENTS
  ${LLVM_TARGETS_TO_BUILD}
  asmparser
  support
  mc
  )

set(SOURCES
  ARCMigrate.cpp
  CIndex.cpp
  CIndexCXX.cpp
  CIndexCodeCompletion.cpp
  CIndexDiagnostic.cpp
  CIndexDiagnostic.h
  CIndexHigh.cpp
  CIndexInclusionStack.cpp
  CIndexUSRs.cpp
  CIndexer.cpp
  CIndexer.h
  CXComment.cpp
  CXCursor.cpp
  CXCursor.h
  CXCompilationDatabase.cpp
  CXLoadedDiagnostic.cpp
  CXLoadedDiagnostic.h
  CXSourceLocation.cpp
  CXSourceLocation.h
  CXStoredDiagnostic.cpp
  CXString.cpp
  CXString.h
  CXTranslationUnit.h
  CXType.cpp
  CXType.h
  IndexBody.cpp
  IndexDecl.cpp
  IndexTypeSourceInfo.cpp
  Index_Internal.h
  Indexing.cpp
  IndexingContext.cpp
  IndexingContext.h
  SimpleFormatContext.h
  ../../include/lfort-c/Index.h
  )

set(LIBRARIES
  lfortARCMigrate
  lfortRewriteCore
  lfortRewriteFrontend
  lfortFrontend
  lfortDriver
  lfortSerialization
  lfortSema
  lfortEdit
  lfortAST
  lfortLex
  lfortTooling
  lfortBasic
  lfortFormat
  )

set(GENERATED_HEADERS
  LFortAttrClasses
  LFortAttrList
  LFortAttrParsedAttrList
  LFortCommentNodes
  LFortDiagnosticCommon
  LFortDiagnosticFrontend
  LFortDeclNodes
  LFortStmtNodes
  )

if( LLVM_ENABLE_PIC )
  set(SHARED_LIBRARY TRUE)
  add_lfort_library(liblfort ${SOURCES})
  target_link_libraries(liblfort ${LIBRARIES})
  add_dependencies(liblfort ${GENERATED_HEADERS})

  if(WIN32)
    set_target_properties(liblfort
      PROPERTIES
      OUTPUT_NAME "liblfort"
      VERSION ${LIBLFORT_LIBRARY_VERSION}
      DEFINE_SYMBOL _CINDEX_LIB_)
  else()
    set_target_properties(liblfort
      PROPERTIES
      OUTPUT_NAME "lfort"
      VERSION ${LIBLFORT_LIBRARY_VERSION}
      DEFINE_SYMBOL _CINDEX_LIB_)
  endif()

  if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(LIBLFORT_LINK_FLAGS
      "-Wl,-compatibility_version -Wl,1 -Wl,-dead_strip")
    set_target_properties(liblfort
      PROPERTIES
      LINK_FLAGS "${LIBLFORT_LINK_FLAGS}"
      INSTALL_NAME_DIR "@executable_path/../lib")
  endif()


  set(LIBLFORT_STATIC_TARGET_NAME liblfort_static)
else()
  set(LIBLFORT_STATIC_TARGET_NAME liblfort)
endif()

if( NOT BUILD_SHARED_LIBS AND NOT WIN32 )
  add_lfort_library(${LIBLFORT_STATIC_TARGET_NAME} STATIC ${SOURCES})
  target_link_libraries(${LIBLFORT_STATIC_TARGET_NAME} ${LIBRARIES})
  add_dependencies(${LIBLFORT_STATIC_TARGET_NAME} ${GENERATED_HEADERS})

  set_target_properties(${LIBLFORT_STATIC_TARGET_NAME}
    PROPERTIES
    OUTPUT_NAME "lfort")
endif()
