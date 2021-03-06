//===--- FrontendActions.cpp ----------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Frontend/FrontendActions.h"
#include "lfort/AST/ASTConsumer.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Frontend/ASTConsumers.h"
#include "lfort/Frontend/ASTUnit.h"
#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/Frontend/Utils.h"
#include "lfort/Lex/HeaderSearch.h"
#include "lfort/Lex/Pragma.h"
#include "lfort/Lex/Preprocessor.h"
#include "lfort/Parse/Parser.h"
#include "lfort/Serialization/ASTWriter.h"
#include "llvm/ADT/OwningPtr.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

using namespace lfort;

//===----------------------------------------------------------------------===//
// Custom Actions
//===----------------------------------------------------------------------===//

ASTConsumer *InitOnlyAction::CreateASTConsumer(CompilerInstance &CI,
                                               StringRef InFile) {
  return new ASTConsumer();
}

void InitOnlyAction::ExecuteAction() {
}

//===----------------------------------------------------------------------===//
// AST Consumer Actions
//===----------------------------------------------------------------------===//

ASTConsumer *ASTPrintAction::CreateASTConsumer(CompilerInstance &CI,
                                               StringRef InFile) {
  if (raw_ostream *OS = CI.createDefaultOutputFile(false, InFile))
    return CreateASTPrinter(OS, CI.getFrontendOpts().ASTDumpFilter);
  return 0;
}

ASTConsumer *ASTDumpAction::CreateASTConsumer(CompilerInstance &CI,
                                              StringRef InFile) {
  return CreateASTDumper(CI.getFrontendOpts().ASTDumpFilter);
}

ASTConsumer *ASTDeclListAction::CreateASTConsumer(CompilerInstance &CI,
                                                  StringRef InFile) {
  return CreateASTDeclNodeLister();
}

ASTConsumer *ASTDumpXMLAction::CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) {
  raw_ostream *OS;
  if (CI.getFrontendOpts().OutputFile.empty())
    OS = &llvm::outs();
  else
    OS = CI.createDefaultOutputFile(false, InFile);
  if (!OS) return 0;
  return CreateASTDumperXML(*OS);
}

ASTConsumer *ASTViewAction::CreateASTConsumer(CompilerInstance &CI,
                                              StringRef InFile) {
  return CreateASTViewer();
}

ASTConsumer *DeclContextPrintAction::CreateASTConsumer(CompilerInstance &CI,
                                                       StringRef InFile) {
  return CreateDeclContextPrinter();
}

ASTConsumer *GeneratePCHAction::CreateASTConsumer(CompilerInstance &CI,
                                                  StringRef InFile) {
  std::string Sysroot;
  std::string OutputFile;
  raw_ostream *OS = 0;
  if (ComputeASTConsumerArguments(CI, InFile, Sysroot, OutputFile, OS))
    return 0;

  if (!CI.getFrontendOpts().RelocatablePCH)
    Sysroot.clear();
  return new PCHGenerator(CI.getPreprocessor(), OutputFile, 0, Sysroot, OS);
}

bool GeneratePCHAction::ComputeASTConsumerArguments(CompilerInstance &CI,
                                                    StringRef InFile,
                                                    std::string &Sysroot,
                                                    std::string &OutputFile,
                                                    raw_ostream *&OS) {
  Sysroot = CI.getHeaderSearchOpts().Sysroot;
  if (CI.getFrontendOpts().RelocatablePCH && Sysroot.empty()) {
    CI.getDiagnostics().Report(diag::err_relocatable_without_isysroot);
    return true;
  }

  // We use createOutputFile here because this is exposed via liblfort, and we
  // must disable the RemoveFileOnSignal behavior.
  // We use a temporary to avoid race conditions.
  OS = CI.createOutputFile(CI.getFrontendOpts().OutputFile, /*Binary=*/true,
                           /*RemoveFileOnSignal=*/false, InFile,
                           /*Extension=*/"", /*useTemporary=*/true);
  if (!OS)
    return true;

  OutputFile = CI.getFrontendOpts().OutputFile;
  return false;
}

ASTConsumer *GeneratePCModuleAction::CreateASTConsumer(CompilerInstance &CI,
                                                     StringRef InFile) {
  std::string Sysroot;
  std::string OutputFile;
  raw_ostream *OS = 0;
  if (ComputeASTConsumerArguments(CI, InFile, Sysroot, OutputFile, OS))
    return 0;
  
  return new PCHGenerator(CI.getPreprocessor(), OutputFile, PCModule, 
                          Sysroot, OS);
}

static SmallVectorImpl<char> &
operator+=(SmallVectorImpl<char> &Includes, StringRef RHS) {
  Includes.append(RHS.begin(), RHS.end());
  return Includes;
}

static void addHeaderInclude(StringRef HeaderName,
                             SmallVectorImpl<char> &Includes,
                             const LangOptions &LangOpts) {
  if (LangOpts.ObjC1)
    Includes += "#import \"";
  else
    Includes += "#include \"";
  Includes += HeaderName;
  Includes += "\"\n";
}

static void addHeaderInclude(const FileEntry *Header,
                             SmallVectorImpl<char> &Includes,
                             const LangOptions &LangOpts) {
  addHeaderInclude(Header->getName(), Includes, LangOpts);
}

/// \brief Collect the set of header includes needed to construct the given 
/// module and update the TopHeaders file set of the module.
///
/// \param PCModule The module we're collecting includes from.
///
/// \param Includes Will be augmented with the set of \#includes or \#imports
/// needed to load all of the named headers.
static void collectPCModuleHeaderIncludes(const LangOptions &LangOpts,
                                        FileManager &FileMgr,
                                        PCModuleMap &ModMap,
                                        lfort::PCModule *PCModule,
                                        SmallVectorImpl<char> &Includes) {
  // Don't collect any headers for unavailable modules.
  if (!PCModule->isAvailable())
    return;

  // Add includes for each of these headers.
  for (unsigned I = 0, N = PCModule->Headers.size(); I != N; ++I) {
    const FileEntry *Header = PCModule->Headers[I];
    PCModule->TopHeaders.insert(Header);
    addHeaderInclude(Header, Includes, LangOpts);
  }

  if (const FileEntry *UmbrellaHeader = PCModule->getUmbrellaHeader()) {
    PCModule->TopHeaders.insert(UmbrellaHeader);
    if (PCModule->Parent) {
      // Include the umbrella header for submodules.
      addHeaderInclude(UmbrellaHeader, Includes, LangOpts);
    }
  } else if (const DirectoryEntry *UmbrellaDir = PCModule->getUmbrellaDir()) {
    // Add all of the headers we find in this subdirectory.
    llvm::error_code EC;
    SmallString<128> DirNative;
    llvm::sys::path::native(UmbrellaDir->getName(), DirNative);
    for (llvm::sys::fs::recursive_directory_iterator Dir(DirNative.str(), EC), 
                                                     DirEnd;
         Dir != DirEnd && !EC; Dir.increment(EC)) {
      // Check whether this entry has an extension typically associated with 
      // headers.
      if (!llvm::StringSwitch<bool>(llvm::sys::path::extension(Dir->path()))
          .Cases(".h", ".H", ".hh", ".hpp", true)
          .Default(false))
        continue;
      
      // If this header is marked 'unavailable' in this module, don't include 
      // it.
      if (const FileEntry *Header = FileMgr.getFile(Dir->path())) {
        if (ModMap.isHeaderInUnavailablePCModule(Header))
          continue;
        PCModule->TopHeaders.insert(Header);
      }
      
      // Include this header umbrella header for submodules.
      addHeaderInclude(Dir->path(), Includes, LangOpts);
    }
  }
  
  // Recurse into submodules.
  for (lfort::PCModule::submodule_iterator Sub = PCModule->submodule_begin(),
                                      SubEnd = PCModule->submodule_end();
       Sub != SubEnd; ++Sub)
    collectPCModuleHeaderIncludes(LangOpts, FileMgr, ModMap, *Sub, Includes);
}

bool GeneratePCModuleAction::BeginSourceFileAction(CompilerInstance &CI, 
                                                 StringRef Filename) {
  // Find the module map file.  
  const FileEntry *PCModuleMap = CI.getFileManager().getFile(Filename);
  if (!PCModuleMap)  {
    CI.getDiagnostics().Report(diag::err_module_map_not_found)
      << Filename;
    return false;
  }
  
  // Parse the module map file.
  HeaderSearch &HS = CI.getPreprocessor().getHeaderSearchInfo();
  if (HS.loadPCModuleMapFile(PCModuleMap))
    return false;
  
  if (CI.getLangOpts().CurrentPCModule.empty()) {
    CI.getDiagnostics().Report(diag::err_missing_module_name);
    
    // FIXME: Eventually, we could consider asking whether there was just
    // a single module described in the module map, and use that as a 
    // default. Then it would be fairly trivial to just "compile" a module
    // map with a single module (the common case).
    return false;
  }
  
  // Dig out the module definition.
  PCModule = HS.lookupPCModule(CI.getLangOpts().CurrentPCModule, 
                           /*AllowSearch=*/false);
  if (!PCModule) {
    CI.getDiagnostics().Report(diag::err_missing_module)
      << CI.getLangOpts().CurrentPCModule << Filename;
    
    return false;
  }

  // Check whether we can build this module at all.
  StringRef Feature;
  if (!PCModule->isAvailable(CI.getLangOpts(), CI.getTarget(), Feature)) {
    CI.getDiagnostics().Report(diag::err_module_unavailable)
      << PCModule->getFullPCModuleName()
      << Feature;

    return false;
  }

  FileManager &FileMgr = CI.getFileManager();

  // Collect the set of #includes we need to build the module.
  SmallString<256> HeaderContents;
  if (const FileEntry *UmbrellaHeader = PCModule->getUmbrellaHeader())
    addHeaderInclude(UmbrellaHeader, HeaderContents, CI.getLangOpts());
  collectPCModuleHeaderIncludes(CI.getLangOpts(), FileMgr,
    CI.getPreprocessor().getHeaderSearchInfo().getPCModuleMap(),
    PCModule, HeaderContents);

  llvm::MemoryBuffer *InputBuffer =
      llvm::MemoryBuffer::getMemBufferCopy(HeaderContents,
                                           PCModule::getPCModuleInputBufferName());
  // Ownership of InputBuffer will be transfered to the SourceManager.
  setCurrentInput(FrontendInputFile(InputBuffer, getCurrentFileKind(),
                                    PCModule->IsSystem));
  return true;
}

bool GeneratePCModuleAction::ComputeASTConsumerArguments(CompilerInstance &CI,
                                                       StringRef InFile,
                                                       std::string &Sysroot,
                                                       std::string &OutputFile,
                                                       raw_ostream *&OS) {
  // If no output file was provided, figure out where this module would go
  // in the module cache.
  if (CI.getFrontendOpts().OutputFile.empty()) {
    HeaderSearch &HS = CI.getPreprocessor().getHeaderSearchInfo();
    SmallString<256> PCModuleFileName(HS.getPCModuleCachePath());
    llvm::sys::path::append(PCModuleFileName, 
                            CI.getLangOpts().CurrentPCModule + ".pcm");
    CI.getFrontendOpts().OutputFile = PCModuleFileName.str();
  }
  
  // We use createOutputFile here because this is exposed via liblfort, and we
  // must disable the RemoveFileOnSignal behavior.
  // We use a temporary to avoid race conditions.
  OS = CI.createOutputFile(CI.getFrontendOpts().OutputFile, /*Binary=*/true,
                           /*RemoveFileOnSignal=*/false, InFile,
                           /*Extension=*/"", /*useTemporary=*/true,
                           /*CreateMissingDirectories=*/true);
  if (!OS)
    return true;
  
  OutputFile = CI.getFrontendOpts().OutputFile;
  return false;
}

ASTConsumer *SyntaxOnlyAction::CreateASTConsumer(CompilerInstance &CI,
                                                 StringRef InFile) {
  return new ASTConsumer();
}

//===----------------------------------------------------------------------===//
// Preprocessor Actions
//===----------------------------------------------------------------------===//

void DumpRawTokensAction::ExecuteAction() {
  Preprocessor &PP = getCompilerInstance().getPreprocessor();
  SourceManager &SM = PP.getSourceManager();

  // Start lexing the specified input file.
  const llvm::MemoryBuffer *FromFile = SM.getBuffer(SM.getMainFileID());
  Lexer RawLex(SM.getMainFileID(), FromFile, SM, PP.getLangOpts());
  RawLex.SetKeepWhitespaceMode(true);

  Token RawTok;
  RawLex.LexFromRawLexer(RawTok);
  while (RawTok.isNot(tok::eof)) {
    PP.DumpToken(RawTok, true);
    llvm::errs() << "\n";
    RawLex.LexFromRawLexer(RawTok);
  }
}

void DumpTokensAction::ExecuteAction() {
  Preprocessor &PP = getCompilerInstance().getPreprocessor();
  // Start preprocessing the specified input file.
  Token Tok;
  PP.EnterMainSourceFile();
  do {
    PP.Lex(Tok);
    PP.DumpToken(Tok, true);
    llvm::errs() << "\n";
  } while (Tok.isNot(tok::eof));
}

void GeneratePTHAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  if (CI.getFrontendOpts().OutputFile.empty() ||
      CI.getFrontendOpts().OutputFile == "-") {
    // FIXME: Don't fail this way.
    // FIXME: Verify that we can actually seek in the given file.
    llvm::report_fatal_error("PTH requires a seekable file for output!");
  }
  llvm::raw_fd_ostream *OS =
    CI.createDefaultOutputFile(true, getCurrentFile());
  if (!OS) return;

  CacheTokens(CI.getPreprocessor(), OS);
}

void PreprocessOnlyAction::ExecuteAction() {
  Preprocessor &PP = getCompilerInstance().getPreprocessor();

  // Ignore unknown pragmas.
  PP.AddPragmaHandler(new EmptyPragmaHandler());

  Token Tok;
  // Start parsing the specified input file.
  PP.EnterMainSourceFile();
  do {
    PP.Lex(Tok);
  } while (Tok.isNot(tok::eof));
}

void PrintPreprocessedAction::ExecuteAction() {
  CompilerInstance &CI = getCompilerInstance();
  // Output file may need to be set to 'Binary', to avoid converting Unix style
  // line feeds (<LF>) to Microsoft style line feeds (<CR><LF>).
  //
  // Look to see what type of line endings the file uses. If there's a
  // CRLF, then we won't open the file up in binary mode. If there is
  // just an LF or CR, then we will open the file up in binary mode.
  // In this fashion, the output format should match the input format, unless
  // the input format has inconsistent line endings.
  //
  // This should be a relatively fast operation since most files won't have
  // all of their source code on a single line. However, that is still a 
  // concern, so if we scan for too long, we'll just assume the file should
  // be opened in binary mode.
  bool BinaryMode = true;
  bool InvalidFile = false;
  const SourceManager& SM = CI.getSourceManager();
  const llvm::MemoryBuffer *Buffer = SM.getBuffer(SM.getMainFileID(), 
                                                     &InvalidFile);
  if (!InvalidFile) {
    const char *cur = Buffer->getBufferStart();
    const char *end = Buffer->getBufferEnd();
    const char *next = (cur != end) ? cur + 1 : end;

    // Limit ourselves to only scanning 256 characters into the source
    // file.  This is mostly a sanity check in case the file has no 
    // newlines whatsoever.
    if (end - cur > 256) end = cur + 256;
	  
    while (next < end) {
      if (*cur == 0x0D) {  // CR
        if (*next == 0x0A)  // CRLF
          BinaryMode = false;

        break;
      } else if (*cur == 0x0A)  // LF
        break;

      ++cur, ++next;
    }
  }

  raw_ostream *OS = CI.createDefaultOutputFile(BinaryMode, getCurrentFile());
  if (!OS) return;

  DoPrintPreprocessedInput(CI.getPreprocessor(), OS,
                           CI.getPreprocessorOutputOpts());
}

void PrintPreambleAction::ExecuteAction() {
  switch (getCurrentFileKind()) {
  case IK_Fortran77:
  case IK_Fortran90:
  case IK_Fortran95:
  case IK_Fortran03:
  case IK_Fortran08:
    break;
      
  case IK_None:
  case IK_PreprocessedFortran77:
  case IK_PreprocessedFortran90:
  case IK_PreprocessedFortran95:
  case IK_PreprocessedFortran03:
  case IK_PreprocessedFortran08:
  case IK_AST:
  case IK_LLVM_IR:
    // We can't do anything with these.
    return;
  }
  
  CompilerInstance &CI = getCompilerInstance();
  llvm::MemoryBuffer *Buffer
      = CI.getFileManager().getBufferForFile(getCurrentFile());
  if (Buffer) {
    unsigned Preamble = Lexer::ComputePreamble(Buffer, CI.getLangOpts()).first;
    llvm::outs().write(Buffer->getBufferStart(), Preamble);
    delete Buffer;
  }
}
