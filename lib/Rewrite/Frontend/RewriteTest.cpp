//===--- RewriteTest.cpp - Rewriter playground ----------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This is a testbed.
//
//===----------------------------------------------------------------------===//

#include "lfort/Rewrite/Frontend/Rewriters.h"
#include "lfort/Lex/Preprocessor.h"
#include "lfort/Rewrite/Core/TokenRewriter.h"
#include "llvm/Support/raw_ostream.h"

void lfort::DoRewriteTest(Preprocessor &PP, raw_ostream* OS) {
  SourceManager &SM = PP.getSourceManager();
  const LangOptions &LangOpts = PP.getLangOpts();

  TokenRewriter Rewriter(SM.getMainFileID(), SM, LangOpts);

  // Throw <i> </i> tags around comments.
  for (TokenRewriter::token_iterator I = Rewriter.token_begin(),
       E = Rewriter.token_end(); I != E; ++I) {
    if (I->isNot(tok::comment)) continue;

    Rewriter.AddTokenBefore(I, "<i>");
    Rewriter.AddTokenAfter(I, "</i>");
  }


  // Print out the output.
  for (TokenRewriter::token_iterator I = Rewriter.token_begin(),
       E = Rewriter.token_end(); I != E; ++I)
    *OS << PP.getSpelling(*I);
}
