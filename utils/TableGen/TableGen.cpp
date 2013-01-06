//===- TableGen.cpp - Top-Level TableGen implementation for LFort ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the main function for LFort's TableGen.
//
//===----------------------------------------------------------------------===//

#include "TableGenBackends.h" // Declares all backends.
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Error.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"

using namespace llvm;
using namespace lfort;

enum ActionType {
  GenLFortAttrClasses,
  GenLFortAttrImpl,
  GenLFortAttrList,
  GenLFortAttrPCHRead,
  GenLFortAttrPCHWrite,
  GenLFortAttrSpellingList,
  GenLFortAttrLateParsedList,
  GenLFortAttrTemplateInstantiate,
  GenLFortAttrParsedAttrList,
  GenLFortAttrParsedAttrKinds,
  GenLFortDiagsDefs,
  GenLFortDiagGroups,
  GenLFortDiagsIndexName,
  GenLFortCommentNodes,
  GenLFortDeclNodes,
  GenLFortStmtNodes,
  GenLFortSACheckers,
  GenLFortCommentHTMLTags,
  GenLFortCommentHTMLTagsProperties,
  GenLFortCommentCommandInfo,
  GenOptParserDefs, GenOptParserImpl,
  GenArmNeon,
  GenArmNeonSema,
  GenArmNeonTest
};

namespace {
  cl::opt<ActionType>
  Action(cl::desc("Action to perform:"),
         cl::values(clEnumValN(GenOptParserDefs, "gen-opt-parser-defs",
                               "Generate option definitions"),
                    clEnumValN(GenOptParserImpl, "gen-opt-parser-impl",
                               "Generate option parser implementation"),
                    clEnumValN(GenLFortAttrClasses, "gen-lfort-attr-classes",
                               "Generate lfort attribute clases"),
                    clEnumValN(GenLFortAttrImpl, "gen-lfort-attr-impl",
                               "Generate lfort attribute implementations"),
                    clEnumValN(GenLFortAttrList, "gen-lfort-attr-list",
                               "Generate a lfort attribute list"),
                    clEnumValN(GenLFortAttrPCHRead, "gen-lfort-attr-pch-read",
                               "Generate lfort PCH attribute reader"),
                    clEnumValN(GenLFortAttrPCHWrite, "gen-lfort-attr-pch-write",
                               "Generate lfort PCH attribute writer"),
                    clEnumValN(GenLFortAttrSpellingList,
                               "gen-lfort-attr-spelling-list",
                               "Generate a lfort attribute spelling list"),
                    clEnumValN(GenLFortAttrLateParsedList,
                               "gen-lfort-attr-late-parsed-list",
                               "Generate a lfort attribute LateParsed list"),
                    clEnumValN(GenLFortAttrTemplateInstantiate,
                               "gen-lfort-attr-template-instantiate",
                               "Generate a lfort template instantiate code"),
                    clEnumValN(GenLFortAttrParsedAttrList,
                               "gen-lfort-attr-parsed-attr-list",
                               "Generate a lfort parsed attribute list"),
                    clEnumValN(GenLFortAttrParsedAttrKinds,
                               "gen-lfort-attr-parsed-attr-kinds",
                               "Generate a lfort parsed attribute kinds"),
                    clEnumValN(GenLFortDiagsDefs, "gen-lfort-diags-defs",
                               "Generate LFort diagnostics definitions"),
                    clEnumValN(GenLFortDiagGroups, "gen-lfort-diag-groups",
                               "Generate LFort diagnostic groups"),
                    clEnumValN(GenLFortDiagsIndexName,
                               "gen-lfort-diags-index-name",
                               "Generate LFort diagnostic name index"),
                    clEnumValN(GenLFortCommentNodes, "gen-lfort-comment-nodes",
                               "Generate LFort AST comment nodes"),
                    clEnumValN(GenLFortDeclNodes, "gen-lfort-decl-nodes",
                               "Generate LFort AST declaration nodes"),
                    clEnumValN(GenLFortStmtNodes, "gen-lfort-stmt-nodes",
                               "Generate LFort AST statement nodes"),
                    clEnumValN(GenLFortSACheckers, "gen-lfort-sa-checkers",
                               "Generate LFort Static Analyzer checkers"),
                    clEnumValN(GenLFortCommentHTMLTags,
                               "gen-lfort-comment-html-tags",
                               "Generate efficient matchers for HTML tag "
                               "names that are used in documentation comments"),
                    clEnumValN(GenLFortCommentHTMLTagsProperties,
                               "gen-lfort-comment-html-tags-properties",
                               "Generate efficient matchers for HTML tag "
                               "properties"),
                    clEnumValN(GenLFortCommentCommandInfo,
                               "gen-lfort-comment-command-info",
                               "Generate list of commands that are used in "
                               "documentation comments"),
                    clEnumValN(GenArmNeon, "gen-arm-neon",
                               "Generate arm_neon.h for lfort"),
                    clEnumValN(GenArmNeonSema, "gen-arm-neon-sema",
                               "Generate ARM NEON sema support for lfort"),
                    clEnumValN(GenArmNeonTest, "gen-arm-neon-test",
                               "Generate ARM NEON tests for lfort"),
                    clEnumValEnd));

  cl::opt<std::string>
  LFortComponent("lfort-component",
                 cl::desc("Only use warnings from specified component"),
                 cl::value_desc("component"), cl::Hidden);

bool LFortTableGenMain(raw_ostream &OS, RecordKeeper &Records) {
  switch (Action) {
  case GenLFortAttrClasses:
    EmitLFortAttrClass(Records, OS);
    break;
  case GenLFortAttrImpl:
    EmitLFortAttrImpl(Records, OS);
    break;
  case GenLFortAttrList:
    EmitLFortAttrList(Records, OS);
    break;
  case GenLFortAttrPCHRead:
    EmitLFortAttrPCHRead(Records, OS);
    break;
  case GenLFortAttrPCHWrite:
    EmitLFortAttrPCHWrite(Records, OS);
    break;
  case GenLFortAttrSpellingList:
    EmitLFortAttrSpellingList(Records, OS);
    break;
  case GenLFortAttrLateParsedList:
    EmitLFortAttrLateParsedList(Records, OS);
    break;
  case GenLFortAttrTemplateInstantiate:
    EmitLFortAttrTemplateInstantiate(Records, OS);
    break;
  case GenLFortAttrParsedAttrList:
    EmitLFortAttrParsedAttrList(Records, OS);
    break;
  case GenLFortAttrParsedAttrKinds:
    EmitLFortAttrParsedAttrKinds(Records, OS);
    break;
  case GenLFortDiagsDefs:
    EmitLFortDiagsDefs(Records, OS, LFortComponent);
    break;
  case GenLFortDiagGroups:
    EmitLFortDiagGroups(Records, OS);
    break;
  case GenLFortDiagsIndexName:
    EmitLFortDiagsIndexName(Records, OS);
    break;
  case GenLFortCommentNodes:
    EmitLFortASTNodes(Records, OS, "Comment", "");
    break;
  case GenLFortDeclNodes:
    EmitLFortASTNodes(Records, OS, "Decl", "Decl");
    EmitLFortDeclContext(Records, OS);
    break;
  case GenLFortStmtNodes:
    EmitLFortASTNodes(Records, OS, "Stmt", "");
    break;
  case GenLFortSACheckers:
    EmitLFortSACheckers(Records, OS);
    break;
  case GenLFortCommentHTMLTags:
    EmitLFortCommentHTMLTags(Records, OS);
    break;
  case GenLFortCommentHTMLTagsProperties:
    EmitLFortCommentHTMLTagsProperties(Records, OS);
    break;
  case GenLFortCommentCommandInfo:
    EmitLFortCommentCommandInfo(Records, OS);
    break;
  case GenOptParserDefs:
    EmitOptParser(Records, OS, true);
    break;
  case GenOptParserImpl:
    EmitOptParser(Records, OS, false);
    break;
  case GenArmNeon:
    EmitNeon(Records, OS);
    break;
  case GenArmNeonSema:
    EmitNeonSema(Records, OS);
    break;
  case GenArmNeonTest:
    EmitNeonTest(Records, OS);
    break;
  }

  return false;
}
}

int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal();
  PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  return TableGenMain(argv[0], &LFortTableGenMain);
}
