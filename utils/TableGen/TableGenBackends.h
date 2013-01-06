//===- TableGenBackends.h - Declarations for LFort TableGen Backends ------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations for all of the LFort TableGen
// backends. A "TableGen backend" is just a function. See
// "$LLVM_ROOT/utils/TableGen/TableGenBackends.h" for more info.
//
//===----------------------------------------------------------------------===//

#include <string>

namespace llvm {
  class raw_ostream;
  class RecordKeeper;
}

using llvm::raw_ostream;
using llvm::RecordKeeper;

namespace lfort {

void EmitLFortDeclContext(RecordKeeper &RK, raw_ostream &OS);
void EmitLFortASTNodes(RecordKeeper &RK, raw_ostream &OS,
                       const std::string &N, const std::string &S);

void EmitLFortAttrClass(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrImpl(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrList(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrPCHRead(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrPCHWrite(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrSpellingList(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrLateParsedList(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrTemplateInstantiate(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrParsedAttrList(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortAttrParsedAttrKinds(RecordKeeper &Records, raw_ostream &OS);

void EmitLFortDiagsDefs(RecordKeeper &Records, raw_ostream &OS,
                        const std::string &Component);
void EmitLFortDiagGroups(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortDiagsIndexName(RecordKeeper &Records, raw_ostream &OS);

void EmitLFortSACheckers(RecordKeeper &Records, raw_ostream &OS);

void EmitLFortCommentHTMLTags(RecordKeeper &Records, raw_ostream &OS);
void EmitLFortCommentHTMLTagsProperties(RecordKeeper &Records, raw_ostream &OS);

void EmitLFortCommentCommandInfo(RecordKeeper &Records, raw_ostream &OS);

void EmitNeon(RecordKeeper &Records, raw_ostream &OS);
void EmitNeonSema(RecordKeeper &Records, raw_ostream &OS);
void EmitNeonTest(RecordKeeper &Records, raw_ostream &OS);

void EmitOptParser(RecordKeeper &Records, raw_ostream &OS, bool GenDefs);

} // end namespace lfort
