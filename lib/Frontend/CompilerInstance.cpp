//===--- CompilerInstance.cpp ---------------------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "lfort/Frontend/CompilerInstance.h"
#include "lfort/AST/ASTConsumer.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/AST/Decl.h"
#include "lfort/Basic/Diagnostic.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Basic/SourceManager.h"
#include "lfort/Basic/TargetInfo.h"
#include "lfort/Basic/Version.h"
#include "lfort/Frontend/ChainedDiagnosticConsumer.h"
#include "lfort/Frontend/FrontendAction.h"
#include "lfort/Frontend/FrontendActions.h"
#include "lfort/Frontend/FrontendDiagnostic.h"
#include "lfort/Frontend/LogDiagnosticPrinter.h"
#include "lfort/Frontend/SerializedDiagnosticPrinter.h"
#include "lfort/Frontend/TextDiagnosticPrinter.h"
#include "lfort/Frontend/Utils.h"
#include "lfort/Frontend/VerifyDiagnosticConsumer.h"
#include "lfort/Lex/HeaderSearch.h"
#include "lfort/Lex/PTHManager.h"
#include "lfort/Lex/Preprocessor.h"
#include "lfort/Sema/CodeCompleteConsumer.h"
#include "lfort/Sema/Sema.h"
#include "lfort/Serialization/ASTReader.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Config/config.h"
#include "llvm/Support/CrashRecoveryContext.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/LockFileManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/Path.h"
#include "llvm/Support/Program.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/Timer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

using namespace lfort;

CompilerInstance::CompilerInstance()
  : Invocation(new CompilerInvocation()), PCModuleManager(0) {
}

CompilerInstance::~CompilerInstance() {
  assert(OutputFiles.empty() && "Still output files in flight?");
}

void CompilerInstance::setInvocation(CompilerInvocation *Value) {
  Invocation = Value;
}

void CompilerInstance::setDiagnostics(DiagnosticsEngine *Value) {
  Diagnostics = Value;
}

void CompilerInstance::setTarget(TargetInfo *Value) {
  Target = Value;
}

void CompilerInstance::setFileManager(FileManager *Value) {
  FileMgr = Value;
}

void CompilerInstance::setSourceManager(SourceManager *Value) {
  SourceMgr = Value;
}

void CompilerInstance::setPreprocessor(Preprocessor *Value) { PP = Value; }

void CompilerInstance::setASTContext(ASTContext *Value) { Context = Value; }

void CompilerInstance::setSema(Sema *S) {
  TheSema.reset(S);
}

void CompilerInstance::setASTConsumer(ASTConsumer *Value) {
  Consumer.reset(Value);
}

void CompilerInstance::setCodeCompletionConsumer(CodeCompleteConsumer *Value) {
  CompletionConsumer.reset(Value);
}

// Diagnostics
static void SetUpBuildDumpLog(DiagnosticOptions *DiagOpts,
                              unsigned argc, const char* const *argv,
                              DiagnosticsEngine &Diags) {
  std::string ErrorInfo;
  OwningPtr<raw_ostream> OS(
    new llvm::raw_fd_ostream(DiagOpts->DumpBuildInformation.c_str(),ErrorInfo));
  if (!ErrorInfo.empty()) {
    Diags.Report(diag::err_fe_unable_to_open_logfile)
                 << DiagOpts->DumpBuildInformation << ErrorInfo;
    return;
  }

  (*OS) << "lfort -cc1 command line arguments: ";
  for (unsigned i = 0; i != argc; ++i)
    (*OS) << argv[i] << ' ';
  (*OS) << '\n';

  // Chain in a diagnostic client which will log the diagnostics.
  DiagnosticConsumer *Logger =
    new TextDiagnosticPrinter(*OS.take(), DiagOpts, /*OwnsOutputStream=*/true);
  Diags.setClient(new ChainedDiagnosticConsumer(Diags.takeClient(), Logger));
}

static void SetUpDiagnosticLog(DiagnosticOptions *DiagOpts,
                               const CodeGenOptions *CodeGenOpts,
                               DiagnosticsEngine &Diags) {
  std::string ErrorInfo;
  bool OwnsStream = false;
  raw_ostream *OS = &llvm::errs();
  if (DiagOpts->DiagnosticLogFile != "-") {
    // Create the output stream.
    llvm::raw_fd_ostream *FileOS(
      new llvm::raw_fd_ostream(DiagOpts->DiagnosticLogFile.c_str(),
                               ErrorInfo, llvm::raw_fd_ostream::F_Append));
    if (!ErrorInfo.empty()) {
      Diags.Report(diag::warn_fe_cc_log_diagnostics_failure)
        << DiagOpts->DumpBuildInformation << ErrorInfo;
    } else {
      FileOS->SetUnbuffered();
      FileOS->SetUseAtomicWrites(true);
      OS = FileOS;
      OwnsStream = true;
    }
  }

  // Chain in the diagnostic client which will log the diagnostics.
  LogDiagnosticPrinter *Logger = new LogDiagnosticPrinter(*OS, DiagOpts,
                                                          OwnsStream);
  if (CodeGenOpts)
    Logger->setDwarfDebugFlags(CodeGenOpts->DwarfDebugFlags);
  Diags.setClient(new ChainedDiagnosticConsumer(Diags.takeClient(), Logger));
}

static void SetupSerializedDiagnostics(DiagnosticOptions *DiagOpts,
                                       DiagnosticsEngine &Diags,
                                       StringRef OutputFile) {
  std::string ErrorInfo;
  OwningPtr<llvm::raw_fd_ostream> OS;
  OS.reset(new llvm::raw_fd_ostream(OutputFile.str().c_str(), ErrorInfo,
                                    llvm::raw_fd_ostream::F_Binary));
  
  if (!ErrorInfo.empty()) {
    Diags.Report(diag::warn_fe_serialized_diag_failure)
      << OutputFile << ErrorInfo;
    return;
  }
  
  DiagnosticConsumer *SerializedConsumer =
    lfort::serialized_diags::create(OS.take(), DiagOpts);

  
  Diags.setClient(new ChainedDiagnosticConsumer(Diags.takeClient(),
                                                SerializedConsumer));
}

void CompilerInstance::createDiagnostics(int Argc, const char* const *Argv,
                                         DiagnosticConsumer *Client,
                                         bool ShouldOwnClient,
                                         bool ShouldCloneClient) {
  Diagnostics = createDiagnostics(&getDiagnosticOpts(), Argc, Argv, Client,
                                  ShouldOwnClient, ShouldCloneClient,
                                  &getCodeGenOpts());
}

IntrusiveRefCntPtr<DiagnosticsEngine>
CompilerInstance::createDiagnostics(DiagnosticOptions *Opts,
                                    int Argc, const char* const *Argv,
                                    DiagnosticConsumer *Client,
                                    bool ShouldOwnClient,
                                    bool ShouldCloneClient,
                                    const CodeGenOptions *CodeGenOpts) {
  IntrusiveRefCntPtr<DiagnosticIDs> DiagID(new DiagnosticIDs());
  IntrusiveRefCntPtr<DiagnosticsEngine>
      Diags(new DiagnosticsEngine(DiagID, Opts));

  // Create the diagnostic client for reporting errors or for
  // implementing -verify.
  if (Client) {
    if (ShouldCloneClient)
      Diags->setClient(Client->clone(*Diags), ShouldOwnClient);
    else
      Diags->setClient(Client, ShouldOwnClient);
  } else
    Diags->setClient(new TextDiagnosticPrinter(llvm::errs(), Opts));

  // Chain in -verify checker, if requested.
  if (Opts->VerifyDiagnostics)
    Diags->setClient(new VerifyDiagnosticConsumer(*Diags));

  // Chain in -diagnostic-log-file dumper, if requested.
  if (!Opts->DiagnosticLogFile.empty())
    SetUpDiagnosticLog(Opts, CodeGenOpts, *Diags);

  if (!Opts->DumpBuildInformation.empty())
    SetUpBuildDumpLog(Opts, Argc, Argv, *Diags);

  if (!Opts->DiagnosticSerializationFile.empty())
    SetupSerializedDiagnostics(Opts, *Diags,
                               Opts->DiagnosticSerializationFile);
  
  // Configure our handling of diagnostics.
  ProcessWarningOptions(*Diags, *Opts);

  return Diags;
}

// File Manager

void CompilerInstance::createFileManager() {
  FileMgr = new FileManager(getFileSystemOpts());
}

// Source Manager

void CompilerInstance::createSourceManager(FileManager &FileMgr) {
  SourceMgr = new SourceManager(getDiagnostics(), FileMgr);
}

// Preprocessor

void CompilerInstance::createPreprocessor() {
  const PreprocessorOptions &PPOpts = getPreprocessorOpts();

  // Create a PTH manager if we are using some form of a token cache.
  PTHManager *PTHMgr = 0;
  if (!PPOpts.TokenCache.empty())
    PTHMgr = PTHManager::Create(PPOpts.TokenCache, getDiagnostics());

  // Create the Preprocessor.
  HeaderSearch *HeaderInfo = new HeaderSearch(&getHeaderSearchOpts(),
                                              getFileManager(),
                                              getDiagnostics(),
                                              getLangOpts(),
                                              &getTarget());
  PP = new Preprocessor(&getPreprocessorOpts(),
                        getDiagnostics(), getLangOpts(), &getTarget(),
                        getSourceManager(), *HeaderInfo, *this, PTHMgr,
                        /*OwnsHeaderSearch=*/true);

  // Note that this is different then passing PTHMgr to Preprocessor's ctor.
  // That argument is used as the IdentifierInfoLookup argument to
  // IdentifierTable's ctor.
  if (PTHMgr) {
    PTHMgr->setPreprocessor(&*PP);
    PP->setPTHManager(PTHMgr);
  }

  if (PPOpts.DetailedRecord)
    PP->createPreprocessingRecord();

  InitializePreprocessor(*PP, PPOpts, getHeaderSearchOpts(), getFrontendOpts());

  // Set up the module path, including the hash for the
  // module-creation options.
  SmallString<256> SpecificPCModuleCache(
                           getHeaderSearchOpts().PCModuleCachePath);
  if (!getHeaderSearchOpts().DisablePCModuleHash)
    llvm::sys::path::append(SpecificPCModuleCache,
                            getInvocation().getPCModuleHash());
  PP->getHeaderSearchInfo().setPCModuleCachePath(SpecificPCModuleCache);

  // Handle generating dependencies, if requested.
  const DependencyOutputOptions &DepOpts = getDependencyOutputOpts();
  if (!DepOpts.OutputFile.empty())
    AttachDependencyFileGen(*PP, DepOpts);
  if (!DepOpts.DOTOutputFile.empty())
    AttachDependencyGraphGen(*PP, DepOpts.DOTOutputFile,
                             getHeaderSearchOpts().Sysroot);

  
  // Handle generating header include information, if requested.
  if (DepOpts.ShowHeaderIncludes)
    AttachHeaderIncludeGen(*PP);
  if (!DepOpts.HeaderIncludeOutputFile.empty()) {
    StringRef OutputPath = DepOpts.HeaderIncludeOutputFile;
    if (OutputPath == "-")
      OutputPath = "";
    AttachHeaderIncludeGen(*PP, /*ShowAllHeaders=*/true, OutputPath,
                           /*ShowDepth=*/false);
  }
}

// ASTContext

void CompilerInstance::createASTContext() {
  Preprocessor &PP = getPreprocessor();
  Context = new ASTContext(getLangOpts(), PP.getSourceManager(),
                           &getTarget(), PP.getIdentifierTable(),
                           PP.getSelectorTable(), PP.getBuiltinInfo(),
                           /*size_reserve=*/ 0);
}

// ExternalASTSource

void CompilerInstance::createPCHExternalASTSource(StringRef Path,
                                                  bool DisablePCHValidation,
                                                bool AllowPCHWithCompilerErrors,
                                                 void *DeserializationListener){
  OwningPtr<ExternalASTSource> Source;
  bool Preamble = getPreprocessorOpts().PrecompiledPreambleBytes.first != 0;
  Source.reset(createPCHExternalASTSource(Path, getHeaderSearchOpts().Sysroot,
                                          DisablePCHValidation,
                                          AllowPCHWithCompilerErrors,
                                          getPreprocessor(), getASTContext(),
                                          DeserializationListener,
                                          Preamble));
  PCModuleManager = static_cast<ASTReader*>(Source.get());
  getASTContext().setExternalSource(Source);
}

ExternalASTSource *
CompilerInstance::createPCHExternalASTSource(StringRef Path,
                                             const std::string &Sysroot,
                                             bool DisablePCHValidation,
                                             bool AllowPCHWithCompilerErrors,
                                             Preprocessor &PP,
                                             ASTContext &Context,
                                             void *DeserializationListener,
                                             bool Preamble) {
  OwningPtr<ASTReader> Reader;
  Reader.reset(new ASTReader(PP, Context,
                             Sysroot.empty() ? "" : Sysroot.c_str(),
                             DisablePCHValidation,
                             AllowPCHWithCompilerErrors));

  Reader->setDeserializationListener(
            static_cast<ASTDeserializationListener *>(DeserializationListener));
  switch (Reader->ReadAST(Path,
                          Preamble ? serialization::MK_Preamble
                                   : serialization::MK_PCH,
                          SourceLocation(),
                          ASTReader::ARR_None)) {
  case ASTReader::Success:
    // Set the predefines buffer as suggested by the PCH reader. Typically, the
    // predefines buffer will be empty.
    PP.setPredefines(Reader->getSuggestedPredefines());
    return Reader.take();

  case ASTReader::Failure:
    // Unrecoverable failure: don't even try to process the input file.
    break;

  case ASTReader::OutOfDate:
  case ASTReader::VersionMismatch:
  case ASTReader::ConfigurationMismatch:
  case ASTReader::HadErrors:
    // No suitable PCH file could be found. Return an error.
    break;
  }

  return 0;
}

// Code Completion

static bool EnableCodeCompletion(Preprocessor &PP,
                                 const std::string &Filename,
                                 unsigned Line,
                                 unsigned Column) {
  // Tell the source manager to chop off the given file at a specific
  // line and column.
  const FileEntry *Entry = PP.getFileManager().getFile(Filename);
  if (!Entry) {
    PP.getDiagnostics().Report(diag::err_fe_invalid_code_complete_file)
      << Filename;
    return true;
  }

  // Truncate the named file at the given line/column.
  PP.SetCodeCompletionPoint(Entry, Line, Column);
  return false;
}

void CompilerInstance::createCodeCompletionConsumer() {
  const ParsedSourceLocation &Loc = getFrontendOpts().CodeCompletionAt;
  if (!CompletionConsumer) {
    setCodeCompletionConsumer(
      createCodeCompletionConsumer(getPreprocessor(),
                                   Loc.FileName, Loc.Line, Loc.Column,
                                   getFrontendOpts().CodeCompleteOpts,
                                   llvm::outs()));
    if (!CompletionConsumer)
      return;
  } else if (EnableCodeCompletion(getPreprocessor(), Loc.FileName,
                                  Loc.Line, Loc.Column)) {
    setCodeCompletionConsumer(0);
    return;
  }

  if (CompletionConsumer->isOutputBinary() &&
      llvm::sys::Program::ChangeStdoutToBinary()) {
    getPreprocessor().getDiagnostics().Report(diag::err_fe_stdout_binary);
    setCodeCompletionConsumer(0);
  }
}

void CompilerInstance::createFrontendTimer() {
  FrontendTimer.reset(new llvm::Timer("LFort front-end timer"));
}

CodeCompleteConsumer *
CompilerInstance::createCodeCompletionConsumer(Preprocessor &PP,
                                               const std::string &Filename,
                                               unsigned Line,
                                               unsigned Column,
                                               const CodeCompleteOptions &Opts,
                                               raw_ostream &OS) {
  if (EnableCodeCompletion(PP, Filename, Line, Column))
    return 0;

  // Set up the creation routine for code-completion.
  return new PrintingCodeCompleteConsumer(Opts, OS);
}

void CompilerInstance::createSema(ProgramKind PgmKind,
                                  CodeCompleteConsumer *CompletionConsumer) {
  TheSema.reset(new Sema(getPreprocessor(), getASTContext(), getASTConsumer(),
                         PgmKind, CompletionConsumer));
}

// Output Files

void CompilerInstance::addOutputFile(const OutputFile &OutFile) {
  assert(OutFile.OS && "Attempt to add empty stream to output list!");
  OutputFiles.push_back(OutFile);
}

void CompilerInstance::clearOutputFiles(bool EraseFiles) {
  for (std::list<OutputFile>::iterator
         it = OutputFiles.begin(), ie = OutputFiles.end(); it != ie; ++it) {
    delete it->OS;
    if (!it->TempFilename.empty()) {
      if (EraseFiles) {
        bool existed;
        llvm::sys::fs::remove(it->TempFilename, existed);
      } else {
        SmallString<128> NewOutFile(it->Filename);

        // If '-working-directory' was passed, the output filename should be
        // relative to that.
        FileMgr->FixupRelativePath(NewOutFile);
        if (llvm::error_code ec = llvm::sys::fs::rename(it->TempFilename,
                                                        NewOutFile.str())) {
          getDiagnostics().Report(diag::err_unable_to_rename_temp)
            << it->TempFilename << it->Filename << ec.message();

          bool existed;
          llvm::sys::fs::remove(it->TempFilename, existed);
        }
      }
    } else if (!it->Filename.empty() && EraseFiles)
      llvm::sys::Path(it->Filename).eraseFromDisk();

  }
  OutputFiles.clear();
}

llvm::raw_fd_ostream *
CompilerInstance::createDefaultOutputFile(bool Binary,
                                          StringRef InFile,
                                          StringRef Extension) {
  return createOutputFile(getFrontendOpts().OutputFile, Binary,
                          /*RemoveFileOnSignal=*/true, InFile, Extension,
                          /*UseTemporary=*/true);
}

llvm::raw_fd_ostream *
CompilerInstance::createOutputFile(StringRef OutputPath,
                                   bool Binary, bool RemoveFileOnSignal,
                                   StringRef InFile,
                                   StringRef Extension,
                                   bool UseTemporary,
                                   bool CreateMissingDirectories) {
  std::string Error, OutputPathName, TempPathName;
  llvm::raw_fd_ostream *OS = createOutputFile(OutputPath, Error, Binary,
                                              RemoveFileOnSignal,
                                              InFile, Extension,
                                              UseTemporary,
                                              CreateMissingDirectories,
                                              &OutputPathName,
                                              &TempPathName);
  if (!OS) {
    getDiagnostics().Report(diag::err_fe_unable_to_open_output)
      << OutputPath << Error;
    return 0;
  }

  // Add the output file -- but don't try to remove "-", since this means we are
  // using stdin.
  addOutputFile(OutputFile((OutputPathName != "-") ? OutputPathName : "",
                TempPathName, OS));

  return OS;
}

llvm::raw_fd_ostream *
CompilerInstance::createOutputFile(StringRef OutputPath,
                                   std::string &Error,
                                   bool Binary,
                                   bool RemoveFileOnSignal,
                                   StringRef InFile,
                                   StringRef Extension,
                                   bool UseTemporary,
                                   bool CreateMissingDirectories,
                                   std::string *ResultPathName,
                                   std::string *TempPathName) {
  assert((!CreateMissingDirectories || UseTemporary) &&
         "CreateMissingDirectories is only allowed when using temporary files");

  std::string OutFile, TempFile;
  if (!OutputPath.empty()) {
    OutFile = OutputPath;
  } else if (InFile == "-") {
    OutFile = "-";
  } else if (!Extension.empty()) {
    llvm::sys::Path Path(InFile);
    Path.eraseSuffix();
    Path.appendSuffix(Extension);
    OutFile = Path.str();
  } else {
    OutFile = "-";
  }

  OwningPtr<llvm::raw_fd_ostream> OS;
  std::string OSFile;

  if (UseTemporary && OutFile != "-") {
    // Only create the temporary if the parent directory exists (or create
    // missing directories is true) and we can actually write to OutPath,
    // otherwise we want to fail early.
    SmallString<256> AbsPath(OutputPath);
    llvm::sys::fs::make_absolute(AbsPath);
    llvm::sys::Path OutPath(AbsPath);
    bool ParentExists = false;
    if (llvm::sys::fs::exists(llvm::sys::path::parent_path(AbsPath.str()),
                              ParentExists))
      ParentExists = false;
    bool Exists;
    if ((CreateMissingDirectories || ParentExists) &&
        ((llvm::sys::fs::exists(AbsPath.str(), Exists) || !Exists) ||
         (OutPath.isRegularFile() && OutPath.canWrite()))) {
      // Create a temporary file.
      SmallString<128> TempPath;
      TempPath = OutFile;
      TempPath += "-%%%%%%%%";
      int fd;
      if (llvm::sys::fs::unique_file(TempPath.str(), fd, TempPath,
                                     /*makeAbsolute=*/false, 0664)
          == llvm::errc::success) {
        OS.reset(new llvm::raw_fd_ostream(fd, /*shouldClose=*/true));
        OSFile = TempFile = TempPath.str();
      }
    }
  }

  if (!OS) {
    OSFile = OutFile;
    OS.reset(
      new llvm::raw_fd_ostream(OSFile.c_str(), Error,
                               (Binary ? llvm::raw_fd_ostream::F_Binary : 0)));
    if (!Error.empty())
      return 0;
  }

  // Make sure the out stream file gets removed if we crash.
  if (RemoveFileOnSignal)
    llvm::sys::RemoveFileOnSignal(llvm::sys::Path(OSFile));

  if (ResultPathName)
    *ResultPathName = OutFile;
  if (TempPathName)
    *TempPathName = TempFile;

  return OS.take();
}

// Initialization Utilities

bool CompilerInstance::InitializeSourceManager(const FrontendInputFile &Input){
  return InitializeSourceManager(Input, getDiagnostics(),
                                 getFileManager(), getSourceManager(), 
                                 getFrontendOpts());
}

bool CompilerInstance::InitializeSourceManager(const FrontendInputFile &Input,
                                               DiagnosticsEngine &Diags,
                                               FileManager &FileMgr,
                                               SourceManager &SourceMgr,
                                               const FrontendOptions &Opts) {
  SrcMgr::CharacteristicKind
    Kind = Input.isSystem() ? SrcMgr::C_System : SrcMgr::C_User;

  if (Input.isBuffer()) {
    SourceMgr.createMainFileIDForMemBuffer(Input.getBuffer(), Kind);
    assert(!SourceMgr.getMainFileID().isInvalid() &&
           "Couldn't establish MainFileID!");
    return true;
  }

  StringRef InputFile = Input.getFile();

  // Figure out where to get and map in the main file.
  if (InputFile != "-") {
    const FileEntry *File = FileMgr.getFile(InputFile);
    if (!File) {
      Diags.Report(diag::err_fe_error_reading) << InputFile;
      return false;
    }

    // The natural SourceManager infrastructure can't currently handle named
    // pipes, but we would at least like to accept them for the main
    // file. Detect them here, read them with the more generic MemoryBuffer
    // function, and simply override their contents as we do for STDIN.
    if (File->isNamedPipe()) {
      OwningPtr<llvm::MemoryBuffer> MB;
      if (llvm::error_code ec = llvm::MemoryBuffer::getFile(InputFile, MB)) {
        Diags.Report(diag::err_cannot_open_file) << InputFile << ec.message();
        return false;
      }

      // Create a new virtual file that will have the correct size.
      File = FileMgr.getVirtualFile(InputFile, MB->getBufferSize(), 0);
      SourceMgr.overrideFileContents(File, MB.take());
    }

    SourceMgr.createMainFileID(File, Kind);
  } else {
    OwningPtr<llvm::MemoryBuffer> SB;
    if (llvm::MemoryBuffer::getSTDIN(SB)) {
      // FIXME: Give ec.message() in this diag.
      Diags.Report(diag::err_fe_error_reading_stdin);
      return false;
    }
    const FileEntry *File = FileMgr.getVirtualFile(SB->getBufferIdentifier(),
                                                   SB->getBufferSize(), 0);
    SourceMgr.createMainFileID(File, Kind);
    SourceMgr.overrideFileContents(File, SB.take());
  }

  assert(!SourceMgr.getMainFileID().isInvalid() &&
         "Couldn't establish MainFileID!");
  return true;
}

// High-Level Operations

bool CompilerInstance::ExecuteAction(FrontendAction &Act) {
  assert(hasDiagnostics() && "Diagnostics engine is not initialized!");
  assert(!getFrontendOpts().ShowHelp && "Client must handle '-help'!");
  assert(!getFrontendOpts().ShowVersion && "Client must handle '-version'!");

  // FIXME: Take this as an argument, once all the APIs we used have moved to
  // taking it as an input instead of hard-coding llvm::errs.
  raw_ostream &OS = llvm::errs();

  // Create the target instance.
  setTarget(TargetInfo::CreateTargetInfo(getDiagnostics(), &getTargetOpts()));
  if (!hasTarget())
    return false;

  // Inform the target of the language options.
  //
  // FIXME: We shouldn't need to do this, the target should be immutable once
  // created. This complexity should be lifted elsewhere.
  getTarget().setForcedLangOptions(getLangOpts());

  // rewriter project will change target built-in bool type from its default. 
  if (getFrontendOpts().ProgramAction == frontend::RewriteObjC)
    getTarget().noSignedCharForObjCBool();

  // Validate/process some options.
  if (getHeaderSearchOpts().Verbose)
    OS << "lfort -cc1 version " LFORT_VERSION_STRING
       << " based upon " << PACKAGE_STRING
       << " default target " << llvm::sys::getDefaultTargetTriple() << "\n";

  if (getFrontendOpts().ShowTimers)
    createFrontendTimer();

  if (getFrontendOpts().ShowStats)
    llvm::EnableStatistics();

  for (unsigned i = 0, e = getFrontendOpts().Inputs.size(); i != e; ++i) {
    // Reset the ID tables if we are reusing the SourceManager.
    if (hasSourceManager())
      getSourceManager().clearIDTables();

    if (Act.BeginSourceFile(*this, getFrontendOpts().Inputs[i])) {
      Act.Execute();
      Act.EndSourceFile();
    }
  }

  // Notify the diagnostic client that all files were processed.
  getDiagnostics().getClient()->finish();

  if (getDiagnosticOpts().ShowCarets) {
    // We can have multiple diagnostics sharing one diagnostic client.
    // Get the total number of warnings/errors from the client.
    unsigned NumWarnings = getDiagnostics().getClient()->getNumWarnings();
    unsigned NumErrors = getDiagnostics().getClient()->getNumErrors();

    if (NumWarnings)
      OS << NumWarnings << " warning" << (NumWarnings == 1 ? "" : "s");
    if (NumWarnings && NumErrors)
      OS << " and ";
    if (NumErrors)
      OS << NumErrors << " error" << (NumErrors == 1 ? "" : "s");
    if (NumWarnings || NumErrors)
      OS << " generated.\n";
  }

  if (getFrontendOpts().ShowStats && hasFileManager()) {
    getFileManager().PrintStats();
    OS << "\n";
  }

  return !getDiagnostics().getClient()->getNumErrors();
}

/// \brief Determine the appropriate source input kind based on language
/// options.
static InputKind getSourceInputKindFromOptions(const LangOptions &LangOpts) {
  if (LangOpts.F08)
    return IK_Fortran08;

  if (LangOpts.F03)
    return IK_Fortran03;

  if (LangOpts.F95)
    return IK_Fortran95;

  if (LangOpts.F90)
    return IK_Fortran90;

  return IK_Fortran77;
}

namespace {
  struct CompilePCModuleMapData {
    CompilerInstance &Instance;
    GeneratePCModuleAction &CreatePCModuleAction;
  };
}

/// \brief Helper function that executes the module-generating action under
/// a crash recovery context.
static void doCompileMapPCModule(void *UserData) {
  CompilePCModuleMapData &Data
    = *reinterpret_cast<CompilePCModuleMapData *>(UserData);
  Data.Instance.ExecuteAction(Data.CreatePCModuleAction);
}

/// \brief Compile a module file for the given module, using the options 
/// provided by the importing compiler instance.
static void compilePCModule(CompilerInstance &ImportingInstance,
                          SourceLocation ImportLoc,
                          PCModule *PCModule,
                          StringRef PCModuleFileName) {
  llvm::LockFileManager Locked(PCModuleFileName);
  switch (Locked) {
  case llvm::LockFileManager::LFS_Error:
    return;

  case llvm::LockFileManager::LFS_Owned:
    // We're responsible for building the module ourselves. Do so below.
    break;

  case llvm::LockFileManager::LFS_Shared:
    // Someone else is responsible for building the module. Wait for them to
    // finish.
    Locked.waitForUnlock();
    return;
  }

  PCModuleMap &ModMap 
    = ImportingInstance.getPreprocessor().getHeaderSearchInfo().getPCModuleMap();
    
  // Construct a compiler invocation for creating this module.
  IntrusiveRefCntPtr<CompilerInvocation> Invocation
    (new CompilerInvocation(ImportingInstance.getInvocation()));

  PreprocessorOptions &PPOpts = Invocation->getPreprocessorOpts();
  
  // For any options that aren't intended to affect how a module is built,
  // reset them to their default values.
  Invocation->getLangOpts()->resetNonModularOptions();
  PPOpts.resetNonModularOptions();

  // Note the name of the module we're building.
  Invocation->getLangOpts()->CurrentPCModule = PCModule->getTopLevelPCModuleName();

  // Make sure that the failed-module structure has been allocated in
  // the importing instance, and propagate the pointer to the newly-created
  // instance.
  PreprocessorOptions &ImportingPPOpts
    = ImportingInstance.getInvocation().getPreprocessorOpts();
  if (!ImportingPPOpts.FailedPCModules)
    ImportingPPOpts.FailedPCModules = new PreprocessorOptions::FailedPCModulesSet;
  PPOpts.FailedPCModules = ImportingPPOpts.FailedPCModules;

  // If there is a module map file, build the module using the module map.
  // Set up the inputs/outputs so that we build the module from its umbrella
  // header.
  FrontendOptions &FrontendOpts = Invocation->getFrontendOpts();
  FrontendOpts.OutputFile = PCModuleFileName.str();
  FrontendOpts.DisableFree = false;
  FrontendOpts.Inputs.clear();
  InputKind IK = getSourceInputKindFromOptions(*Invocation->getLangOpts());

  // Get or create the module map that we'll use to build this module.
  SmallString<128> TempPCModuleMapFileName;
  if (const FileEntry *PCModuleMapFile
                                  = ModMap.getContainingPCModuleMapFile(PCModule)) {
    // Use the module map where this module resides.
    FrontendOpts.Inputs.push_back(FrontendInputFile(PCModuleMapFile->getName(), 
                                                    IK));
  } else {
    // Create a temporary module map file.
    TempPCModuleMapFileName = PCModule->Name;
    TempPCModuleMapFileName += "-%%%%%%%%.map";
    int FD;
    if (llvm::sys::fs::unique_file(TempPCModuleMapFileName.str(), FD, 
                                   TempPCModuleMapFileName,
                                   /*makeAbsolute=*/true)
          != llvm::errc::success) {
      ImportingInstance.getDiagnostics().Report(diag::err_module_map_temp_file)
        << TempPCModuleMapFileName;
      return;
    }
    // Print the module map to this file.
    llvm::raw_fd_ostream OS(FD, /*shouldClose=*/true);
    PCModule->print(OS);
    FrontendOpts.Inputs.push_back(
      FrontendInputFile(TempPCModuleMapFileName.str().str(), IK));
  }

  // Don't free the remapped file buffers; they are owned by our caller.
  PPOpts.RetainRemappedFileBuffers = true;
    
  Invocation->getDiagnosticOpts().VerifyDiagnostics = 0;
  assert(ImportingInstance.getInvocation().getPCModuleHash() ==
         Invocation->getPCModuleHash() && "PCModule hash mismatch!");
  
  // Construct a compiler instance that will be used to actually create the
  // module.
  CompilerInstance Instance;
  Instance.setInvocation(&*Invocation);
  Instance.createDiagnostics(/*argc=*/0, /*argv=*/0,
                             &ImportingInstance.getDiagnosticClient(),
                             /*ShouldOwnClient=*/true,
                             /*ShouldCloneClient=*/true);

  // Note that this module is part of the module build stack, so that we
  // can detect cycles in the module graph.
  Instance.createFileManager(); // FIXME: Adopt file manager from importer?
  Instance.createSourceManager(Instance.getFileManager());
  SourceManager &SourceMgr = Instance.getSourceManager();
  SourceMgr.setPCModuleBuildStack(
    ImportingInstance.getSourceManager().getPCModuleBuildStack());
  SourceMgr.pushPCModuleBuildStack(PCModule->getTopLevelPCModuleName(),
    FullSourceLoc(ImportLoc, ImportingInstance.getSourceManager()));


  // Construct a module-generating action.
  GeneratePCModuleAction CreatePCModuleAction;
  
  // Execute the action to actually build the module in-place. Use a separate
  // thread so that we get a stack large enough.
  const unsigned ThreadStackSize = 8 << 20;
  llvm::CrashRecoveryContext CRC;
  CompilePCModuleMapData Data = { Instance, CreatePCModuleAction };
  CRC.RunSafelyOnThread(&doCompileMapPCModule, &Data, ThreadStackSize);
  
  // Delete the temporary module map file.
  // FIXME: Even though we're executing under crash protection, it would still
  // be nice to do this with RemoveFileOnSignal when we can. However, that
  // doesn't make sense for all clients, so clean this up manually.
  Instance.clearOutputFiles(/*EraseFiles=*/true);
  if (!TempPCModuleMapFileName.empty())
    llvm::sys::Path(TempPCModuleMapFileName).eraseFromDisk();
}

PCModuleLoadResult
CompilerInstance::loadPCModule(SourceLocation ImportLoc,
                             PCModuleIdPath Path,
                             PCModule::NameVisibilityKind Visibility,
                             bool IsInclusionDirective) {
  // If we've already handled this import, just return the cached result.
  // This one-element cache is important to eliminate redundant diagnostics
  // when both the preprocessor and parser see the same import declaration.
  if (!ImportLoc.isInvalid() && LastPCModuleImportLoc == ImportLoc) {
    // Make the named module visible.
    if (LastPCModuleImportResult)
      PCModuleManager->makePCModuleVisible(LastPCModuleImportResult, Visibility);
    return LastPCModuleImportResult;
  }
  
  // Determine what file we're searching from.
  StringRef PCModuleName = Path[0].first->getName();
  SourceLocation PCModuleNameLoc = Path[0].second;

  lfort::PCModule *PCModule = 0;
  
  // If we don't already have information on this module, load the module now.
  llvm::DenseMap<const IdentifierInfo *, lfort::PCModule *>::iterator Known
    = KnownPCModules.find(Path[0].first);
  if (Known != KnownPCModules.end()) {
    // Retrieve the cached top-level module.
    PCModule = Known->second;    
  } else if (PCModuleName == getLangOpts().CurrentPCModule) {
    // This is the module we're building. 
    PCModule = PP->getHeaderSearchInfo().getPCModuleMap().findPCModule(PCModuleName);
    Known = KnownPCModules.insert(std::make_pair(Path[0].first, PCModule)).first;
  } else {
    // Search for a module with the given name.
    PCModule = PP->getHeaderSearchInfo().lookupPCModule(PCModuleName);
    std::string PCModuleFileName;
    if (PCModule)
      PCModuleFileName = PP->getHeaderSearchInfo().getPCModuleFileName(PCModule);
    else
      PCModuleFileName = PP->getHeaderSearchInfo().getPCModuleFileName(PCModuleName);

    if (PCModuleFileName.empty()) {
      getDiagnostics().Report(PCModuleNameLoc, diag::err_module_not_found)
        << PCModuleName
        << SourceRange(ImportLoc, PCModuleNameLoc);
      LastPCModuleImportLoc = ImportLoc;
      LastPCModuleImportResult = PCModuleLoadResult();
      return LastPCModuleImportResult;
    }
    
    const FileEntry *PCModuleFile
      = getFileManager().getFile(PCModuleFileName, /*OpenFile=*/false,
                                 /*CacheFailure=*/false);
    bool BuildingPCModule = false;
    if (!PCModuleFile && PCModule) {
      // The module is not cached, but we have a module map from which we can
      // build the module.

      // Check whether there is a cycle in the module graph.
      PCModuleBuildStack Path = getSourceManager().getPCModuleBuildStack();
      PCModuleBuildStack::iterator Pos = Path.begin(), PosEnd = Path.end();
      for (; Pos != PosEnd; ++Pos) {
        if (Pos->first == PCModuleName)
          break;
      }

      if (Pos != PosEnd) {
        SmallString<256> CyclePath;
        for (; Pos != PosEnd; ++Pos) {
          CyclePath += Pos->first;
          CyclePath += " -> ";
        }
        CyclePath += PCModuleName;

        getDiagnostics().Report(PCModuleNameLoc, diag::err_module_cycle)
          << PCModuleName << CyclePath;
        return PCModuleLoadResult();
      }

      // Check whether we have already attempted to build this module (but
      // failed).
      if (getPreprocessorOpts().FailedPCModules &&
          getPreprocessorOpts().FailedPCModules->hasAlreadyFailed(PCModuleName)) {
        getDiagnostics().Report(PCModuleNameLoc, diag::err_module_not_built)
          << PCModuleName
          << SourceRange(ImportLoc, PCModuleNameLoc);

        return PCModuleLoadResult();
      }

      BuildingPCModule = true;
      compilePCModule(*this, PCModuleNameLoc, PCModule, PCModuleFileName);
      PCModuleFile = FileMgr->getFile(PCModuleFileName);

      if (!PCModuleFile)
        getPreprocessorOpts().FailedPCModules->addFailed(PCModuleName);
    }

    if (!PCModuleFile) {
      getDiagnostics().Report(PCModuleNameLoc,
                              BuildingPCModule? diag::err_module_not_built
                                            : diag::err_module_not_found)
        << PCModuleName
        << SourceRange(ImportLoc, PCModuleNameLoc);
      return PCModuleLoadResult();
    }

    // If we don't already have an ASTReader, create one now.
    if (!PCModuleManager) {
      if (!hasASTContext())
        createASTContext();

      std::string Sysroot = getHeaderSearchOpts().Sysroot;
      const PreprocessorOptions &PPOpts = getPreprocessorOpts();
      PCModuleManager = new ASTReader(getPreprocessor(), *Context,
                                    Sysroot.empty() ? "" : Sysroot.c_str(),
                                    PPOpts.DisablePCHValidation);
      if (hasASTConsumer()) {
        PCModuleManager->setDeserializationListener(
          getASTConsumer().GetASTDeserializationListener());
        getASTContext().setASTMutationListener(
          getASTConsumer().GetASTMutationListener());
        getPreprocessor().setPPMutationListener(
          getASTConsumer().GetPPMutationListener());
      }
      OwningPtr<ExternalASTSource> Source;
      Source.reset(PCModuleManager);
      getASTContext().setExternalSource(Source);
      if (hasSema())
        PCModuleManager->InitializeSema(getSema());
      if (hasASTConsumer())
        PCModuleManager->StartProgram(&getASTConsumer());
    }

    // Try to load the module we found.
    unsigned ARRFlags = ASTReader::ARR_None;
    if (PCModule)
      ARRFlags |= ASTReader::ARR_OutOfDate;
    switch (PCModuleManager->ReadAST(PCModuleFile->getName(),
                                   serialization::MK_PCModule, ImportLoc,
                                   ARRFlags)) {
    case ASTReader::Success:
      break;

    case ASTReader::OutOfDate: {
      // The module file is out-of-date. Rebuild it.
      getFileManager().invalidateCache(PCModuleFile);
      bool Existed;
      llvm::sys::fs::remove(PCModuleFileName, Existed);

      // Check whether we have already attempted to build this module (but
      // failed).
      if (getPreprocessorOpts().FailedPCModules &&
          getPreprocessorOpts().FailedPCModules->hasAlreadyFailed(PCModuleName)) {
        getDiagnostics().Report(PCModuleNameLoc, diag::err_module_not_built)
          << PCModuleName
          << SourceRange(ImportLoc, PCModuleNameLoc);

        return PCModuleLoadResult();
      }

      compilePCModule(*this, PCModuleNameLoc, PCModule, PCModuleFileName);

      // Try loading the module again.
      PCModuleFile = FileMgr->getFile(PCModuleFileName);
      if (!PCModuleFile ||
          PCModuleManager->ReadAST(PCModuleFileName,
                                 serialization::MK_PCModule, ImportLoc,
                                 ASTReader::ARR_None) != ASTReader::Success) {
        getPreprocessorOpts().FailedPCModules->addFailed(PCModuleName);
        KnownPCModules[Path[0].first] = 0;
        return PCModuleLoadResult();
      }

      // Okay, we've rebuilt and now loaded the module.
      break;
    }

    case ASTReader::VersionMismatch:
    case ASTReader::ConfigurationMismatch:
    case ASTReader::HadErrors:
      // FIXME: The ASTReader will already have complained, but can we showhorn
      // that diagnostic information into a more useful form?
      KnownPCModules[Path[0].first] = 0;
      return PCModuleLoadResult();

    case ASTReader::Failure:
      // Already complained, but note now that we failed.
      KnownPCModules[Path[0].first] = 0;
      return PCModuleLoadResult();
    }
    
    if (!PCModule) {
      // If we loaded the module directly, without finding a module map first,
      // we'll have loaded the module's information from the module itself.
      PCModule = PP->getHeaderSearchInfo().getPCModuleMap()
                 .findPCModule((Path[0].first->getName()));
    }

    if (PCModule)
      PCModule->setASTFile(PCModuleFile);
    
    // Cache the result of this top-level module lookup for later.
    Known = KnownPCModules.insert(std::make_pair(Path[0].first, PCModule)).first;
  }
  
  // If we never found the module, fail.
  if (!PCModule)
    return PCModuleLoadResult();
  
  // Verify that the rest of the module path actually corresponds to
  // a submodule.
  if (Path.size() > 1) {
    for (unsigned I = 1, N = Path.size(); I != N; ++I) {
      StringRef Name = Path[I].first->getName();
      lfort::PCModule *Sub = PCModule->findSubmodule(Name);
      
      if (!Sub) {
        // Attempt to perform typo correction to find a module name that works.
        llvm::SmallVector<StringRef, 2> Best;
        unsigned BestEditDistance = (std::numeric_limits<unsigned>::max)();
        
        for (lfort::PCModule::submodule_iterator J = PCModule->submodule_begin(), 
                                            JEnd = PCModule->submodule_end();
             J != JEnd; ++J) {
          unsigned ED = Name.edit_distance((*J)->Name,
                                           /*AllowReplacements=*/true,
                                           BestEditDistance);
          if (ED <= BestEditDistance) {
            if (ED < BestEditDistance) {
              Best.clear();
              BestEditDistance = ED;
            }
            
            Best.push_back((*J)->Name);
          }
        }
        
        // If there was a clear winner, user it.
        if (Best.size() == 1) {
          getDiagnostics().Report(Path[I].second, 
                                  diag::err_no_submodule_suggest)
            << Path[I].first << PCModule->getFullPCModuleName() << Best[0]
            << SourceRange(Path[0].second, Path[I-1].second)
            << FixItHint::CreateReplacement(SourceRange(Path[I].second),
                                            Best[0]);
          
          Sub = PCModule->findSubmodule(Best[0]);
        }
      }
      
      if (!Sub) {
        // No submodule by this name. Complain, and don't look for further
        // submodules.
        getDiagnostics().Report(Path[I].second, diag::err_no_submodule)
          << Path[I].first << PCModule->getFullPCModuleName()
          << SourceRange(Path[0].second, Path[I-1].second);
        break;
      }
      
      PCModule = Sub;
    }
  }
  
  // Make the named module visible, if it's not already part of the module
  // we are parsing.
  if (PCModuleName != getLangOpts().CurrentPCModule) {
    if (!PCModule->IsFromPCModuleFile) {
      // We have an umbrella header or directory that doesn't actually include
      // all of the headers within the directory it covers. Complain about
      // this missing submodule and recover by forgetting that we ever saw
      // this submodule.
      // FIXME: Should we detect this at module load time? It seems fairly
      // expensive (and rare).
      getDiagnostics().Report(ImportLoc, diag::warn_missing_submodule)
        << PCModule->getFullPCModuleName()
        << SourceRange(Path.front().second, Path.back().second);
      
      return PCModuleLoadResult(0, true);
    }

    // Check whether this module is available.
    StringRef Feature;
    if (!PCModule->isAvailable(getLangOpts(), getTarget(), Feature)) {
      getDiagnostics().Report(ImportLoc, diag::err_module_unavailable)
        << PCModule->getFullPCModuleName()
        << Feature
        << SourceRange(Path.front().second, Path.back().second);
      LastPCModuleImportLoc = ImportLoc;
      LastPCModuleImportResult = PCModuleLoadResult();
      return PCModuleLoadResult();
    }

    PCModuleManager->makePCModuleVisible(PCModule, Visibility);
  }
  
  // If this module import was due to an inclusion directive, create an 
  // implicit import declaration to capture it in the AST.
  if (IsInclusionDirective && hasASTContext()) {
    ProgramDecl *Pgm = getASTContext().getProgramDecl();
    ImportDecl *ImportD = ImportDecl::CreateImplicit(getASTContext(), Pgm,
                                                     ImportLoc, PCModule,
                                                     Path.back().second);
    Pgm->addDecl(ImportD);
    if (Consumer)
      Consumer->HandleImplicitImportDecl(ImportD);
  }
  
  LastPCModuleImportLoc = ImportLoc;
  LastPCModuleImportResult = PCModuleLoadResult(PCModule, false);
  return LastPCModuleImportResult;
}
