//===--- SimpleFormatContext.h ----------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
/// \file
///
/// \brief Defines a utility class for use of lfort-format in liblfort
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_SIMPLE_FORM_CONTEXT_H
#define LLVM_LFORT_SIMPLE_FORM_CONTEXT_H

#include "lfort/Basic/Diagnostic.h"
#include "lfort/Basic/DiagnosticOptions.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Basic/LangOptions.h"
#include "lfort/Basic/SourceManager.h"
#include "lfort/Rewrite/Core/Rewriter.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/raw_ostream.h"

namespace lfort {

/// \brief A small class to be used by liblfort clients to format
/// a declaration string in memory. This object is instantiated once
/// and used each time a formatting is needed.
class SimpleFormatContext {
public:
  SimpleFormatContext(LangOptions Options)
      : DiagOpts(new DiagnosticOptions()),
        Diagnostics(new DiagnosticsEngine(new DiagnosticIDs, 
                                          DiagOpts.getPtr())),
        Files((FileSystemOptions())),
        Sources(*Diagnostics, Files),
        Rewrite(Sources, Options) {
    Diagnostics->setClient(new IgnoringDiagConsumer, true);
  }

  ~SimpleFormatContext() { }

  FileID createInMemoryFile(StringRef Name, StringRef Content) {
    const llvm::MemoryBuffer *Source =
      llvm::MemoryBuffer::getMemBuffer(Content);
    const FileEntry *Entry =
      Files.getVirtualFile(Name, Source->getBufferSize(), 0);
    Sources.overrideFileContents(Entry, Source, true);
    assert(Entry != NULL);
    return Sources.createFileID(Entry, SourceLocation(), SrcMgr::C_User);
  }

  std::string getRewrittenText(FileID ID) {
    std::string Result;
    llvm::raw_string_ostream OS(Result);
    Rewrite.getEditBuffer(ID).write(OS);
    OS.flush();
    return Result;
  }

  llvm::IntrusiveRefCntPtr<DiagnosticOptions> DiagOpts;
  llvm::IntrusiveRefCntPtr<DiagnosticsEngine> Diagnostics;
  FileManager Files;
  SourceManager Sources;
  Rewriter Rewrite;
};

} // end namespace lfort

#endif
