//===-- ASTMerge.cpp - AST Merging Frontent Action --------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
#include "lfort/Frontend/ASTUnit.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/ASTDiagnostic.h"
#include "lfort/AST/ASTImporter.h"
#include "lfort/Basic/Diagnostic.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/FrontendActions.h"

using namespace lfort;

ASTConsumer *ASTMergeAction::CreateASTConsumer(CompilerInstance &CI,
                                               StringRef InFile) {
  return AdaptedAction->CreateASTConsumer(CI, InFile);
}

bool ASTMergeAction::BeginSourceFileAction(CompilerInstance &CI,
                                           StringRef Filename) {
  // FIXME: This is a hack. We need a better way to communicate the
  // AST file, compiler instance, and file name than member variables
  // of FrontendAction.
  AdaptedAction->setCurrentInput(getCurrentInput(), takeCurrentASTUnit());
  AdaptedAction->setCompilerInstance(&CI);
  return AdaptedAction->BeginSourceFileAction(CI, Filename);
}

void ASTMergeAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  CI.getDiagnostics().getClient()->BeginSourceFile(
                                         CI.getASTContext().getLangOpts());
  CI.getDiagnostics().SetArgToStringFn(&FormatASTNodeDiagnosticArgument,
                                       &CI.getASTContext());
  IntrusiveRefCntPtr<DiagnosticIDs>
      DiagIDs(CI.getDiagnostics().getDiagnosticIDs());
  for (unsigned I = 0, N = ASTFiles.size(); I != N; ++I) {
    IntrusiveRefCntPtr<DiagnosticsEngine>
        Diags(new DiagnosticsEngine(DiagIDs, &CI.getDiagnosticOpts(),
                                    CI.getDiagnostics().getClient(),
                                    /*ShouldOwnClient=*/false));
    ASTUnit *Unit = ASTUnit::LoadFromASTFile(ASTFiles[I], Diags,
                                             CI.getFileSystemOpts(), false);
    if (!Unit)
      continue;

    ASTImporter Importer(CI.getASTContext(), 
                         CI.getFileManager(),
                         Unit->getASTContext(), 
                         Unit->getFileManager(),
                         /*MinimalImport=*/false);

    ProgramDecl *Pgm = Unit->getASTContext().getProgramDecl();
    for (DeclContext::decl_iterator D = Pgm->decls_begin(), 
                                 DEnd = Pgm->decls_end();
         D != DEnd; ++D) {
      // Don't re-import __va_list_tag, __builtin_va_list.
      if (NamedDecl *ND = dyn_cast<NamedDecl>(*D))
        if (IdentifierInfo *II = ND->getIdentifier())
          if (II->isStr("__va_list_tag") || II->isStr("__builtin_va_list"))
            continue;
      
      Importer.Import(*D);
    }

    delete Unit;
  }

  AdaptedAction->ExecuteAction();
  CI.getDiagnostics().getClient()->EndSourceFile();
}

void ASTMergeAction::EndSourceFileAction() {
  return AdaptedAction->EndSourceFileAction();
}

ASTMergeAction::ASTMergeAction(FrontendAction *AdaptedAction,
                               ArrayRef<std::string> ASTFiles)
  : AdaptedAction(AdaptedAction), ASTFiles(ASTFiles.begin(), ASTFiles.end()) {
  assert(AdaptedAction && "ASTMergeAction needs an action to adapt");
}

ASTMergeAction::~ASTMergeAction() { 
  delete AdaptedAction;
}

bool ASTMergeAction::usesPreprocessorOnly() const {
  return AdaptedAction->usesPreprocessorOnly();
}

ProgramKind ASTMergeAction::getProgramKind() {
  return AdaptedAction->getProgramKind();
}

bool ASTMergeAction::hasPCHSupport() const {
  return AdaptedAction->hasPCHSupport();
}

bool ASTMergeAction::hasASTFileSupport() const {
  return AdaptedAction->hasASTFileSupport();
}

bool ASTMergeAction::hasCodeCompletionSupport() const {
  return AdaptedAction->hasCodeCompletionSupport();
}
