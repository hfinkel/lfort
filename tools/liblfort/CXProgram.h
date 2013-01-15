//===- CXProgram.h - Routines for manipulating CXPrograms -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines routines for manipulating CXPrograms.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_CXTRANSLATIONUNIT_H
#define LLVM_LFORT_CXTRANSLATIONUNIT_H

extern "C" {
struct CXProgramImpl {
  void *CIdx;
  void *PgmData;
  void *StringPool;
  void *Diagnostics;
  void *OverridenCursorsPool;
  void *FormatContext;
  unsigned FormatInMemoryUniqueId;
};
}

namespace lfort {
  class ASTUnit;
  class CIndexer;

namespace cxtu {

CXProgramImpl *MakeCXProgram(CIndexer *CIdx, ASTUnit *Pgm);
  
class CXPgmOwner {
  CXProgramImpl *Pgm;
  
public:
  CXPgmOwner(CXProgramImpl *tu) : Pgm(tu) { }
  ~CXPgmOwner();

  CXProgramImpl *getPgm() const { return Pgm; }

  CXProgramImpl *takePgm() {
    CXProgramImpl *retPgm = Pgm;
    Pgm = 0;
    return retPgm;
  }
};


}} // end namespace lfort::cxtu

#endif
