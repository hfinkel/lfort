//===--- ParseAST.h - Define the ParseAST method ----------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the lfort::ParseAST method.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_PARSE_PARSEAST_H
#define LLVM_LFORT_PARSE_PARSEAST_H

#include "lfort/Basic/LangOptions.h"

namespace lfort {
  class Preprocessor;
  class ASTConsumer;
  class ASTContext;
  class CodeCompleteConsumer;
  class Sema;

  /// \brief Parse the entire file specified, notifying the ASTConsumer as
  /// the file is parsed.
  ///
  /// This operation inserts the parsed decls into the translation
  /// unit held by Ctx.
  ///
  /// \param PgmKind The kind of translation unit being parsed.
  ///
  /// \param CompletionConsumer If given, an object to consume code completion
  /// results.
  void ParseAST(Preprocessor &pp, ASTConsumer *C,
                ASTContext &Ctx, bool PrintStats = false,
                ProgramKind PgmKind = PGM_Complete,
                CodeCompleteConsumer *CompletionConsumer = 0,
                bool SkipSubprogramBodies = false);

  /// \brief Parse the main file known to the preprocessor, producing an 
  /// abstract syntax tree.
  void ParseAST(Sema &S, bool PrintStats = false,
                bool SkipSubprogramBodies = false);
  
}  // end namespace lfort

#endif
