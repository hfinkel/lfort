//===--- PCModuleMap.cpp - Describe the layout of modules ---------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the PCModuleMap implementation, which describes the layout
// of a module as it relates to headers.
//
//===----------------------------------------------------------------------===//
#include "lfort/Lex/PCModuleMap.h"
#include "lfort/Basic/Diagnostic.h"
#include "lfort/Basic/DiagnosticOptions.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Basic/TargetInfo.h"
#include "lfort/Basic/TargetOptions.h"
#include "lfort/Lex/LexDiagnostic.h"
#include "lfort/Lex/Lexer.h"
#include "lfort/Lex/LiteralSupport.h"
#include "llvm/ADT/StringRef.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/Allocator.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/PathV2.h"
#include "llvm/Support/raw_ostream.h"
#include <stdlib.h>
using namespace lfort;

PCModule::ExportDecl 
PCModuleMap::resolveExport(PCModule *Mod, 
                         const PCModule::UnresolvedExportDecl &Unresolved,
                         bool Complain) {
  // We may have just a wildcard.
  if (Unresolved.Id.empty()) {
    assert(Unresolved.Wildcard && "Invalid unresolved export");
    return PCModule::ExportDecl(0, true);
  }
  
  // Find the starting module.
  PCModule *Context = lookupPCModuleUnqualified(Unresolved.Id[0].first, Mod);
  if (!Context) {
    if (Complain)
      Diags->Report(Unresolved.Id[0].second, 
                    diag::err_mmap_missing_module_unqualified)
        << Unresolved.Id[0].first << Mod->getFullPCModuleName();
    
    return PCModule::ExportDecl();
  }

  // Dig into the module path.
  for (unsigned I = 1, N = Unresolved.Id.size(); I != N; ++I) {
    PCModule *Sub = lookupPCModuleQualified(Unresolved.Id[I].first,
                                        Context);
    if (!Sub) {
      if (Complain)
        Diags->Report(Unresolved.Id[I].second, 
                      diag::err_mmap_missing_module_qualified)
          << Unresolved.Id[I].first << Context->getFullPCModuleName()
          << SourceRange(Unresolved.Id[0].second, Unresolved.Id[I-1].second);
      
      return PCModule::ExportDecl();      
    }
    
    Context = Sub;
  }
  
  return PCModule::ExportDecl(Context, Unresolved.Wildcard);
}

PCModuleMap::PCModuleMap(FileManager &FileMgr, const DiagnosticConsumer &DC,
                     const LangOptions &LangOpts, const TargetInfo *Target)
  : LangOpts(LangOpts), Target(Target), BuiltinIncludeDir(0)
{
  IntrusiveRefCntPtr<DiagnosticIDs> DiagIDs(new DiagnosticIDs);
  Diags = IntrusiveRefCntPtr<DiagnosticsEngine>(
            new DiagnosticsEngine(DiagIDs, new DiagnosticOptions));
  Diags->setClient(DC.clone(*Diags), /*ShouldOwnClient=*/true);
  SourceMgr = new SourceManager(*Diags, FileMgr);
}

PCModuleMap::~PCModuleMap() {
  for (llvm::StringMap<PCModule *>::iterator I = PCModules.begin(), 
                                        IEnd = PCModules.end();
       I != IEnd; ++I) {
    delete I->getValue();
  }
  
  delete SourceMgr;
}

void PCModuleMap::setTarget(const TargetInfo &Target) {
  assert((!this->Target || this->Target == &Target) && 
         "Improper target override");
  this->Target = &Target;
}

/// \brief "Sanitize" a filename so that it can be used as an identifier.
static StringRef sanitizeFilenameAsIdentifier(StringRef Name,
                                              SmallVectorImpl<char> &Buffer) {
  if (Name.empty())
    return Name;

  // Check whether the filename is already an identifier; this is the common
  // case.
  bool isIdentifier = true;
  for (unsigned I = 0, N = Name.size(); I != N; ++I) {
    if (isalpha(Name[I]) || Name[I] == '_' || (isdigit(Name[I]) && I > 0))
      continue;

    isIdentifier = false;
    break;
  }

  if (!isIdentifier) {
    // If we don't already have something with the form of an identifier,
    // create a buffer with the sanitized name.
    Buffer.clear();
    if (isdigit(Name[0]))
      Buffer.push_back('_');
    Buffer.reserve(Buffer.size() + Name.size());
    for (unsigned I = 0, N = Name.size(); I != N; ++I) {
      if (isalnum(Name[I]) || isspace(Name[I]))
        Buffer.push_back(Name[I]);
      else
        Buffer.push_back('_');
    }

    Name = StringRef(Buffer.data(), Buffer.size());
  }

  while (llvm::StringSwitch<bool>(Name)
#define KEYWORD(Keyword,Conditions) .Case(#Keyword, true)
#define ALIAS(Keyword, AliasOf, Conditions) .Case(Keyword, true)
#include "lfort/Basic/TokenKinds.def"
           .Default(false)) {
    if (Name.data() != Buffer.data())
      Buffer.append(Name.begin(), Name.end());
    Buffer.push_back('_');
    Name = StringRef(Buffer.data(), Buffer.size());
  }

  return Name;
}

PCModule *PCModuleMap::findPCModuleForHeader(const FileEntry *File) {
  HeadersMap::iterator Known = Headers.find(File);
  if (Known != Headers.end()) {
    // If a header is not available, don't report that it maps to anything.
    if (!Known->second.isAvailable())
      return 0;

    return Known->second.getPCModule();
  }
  
  const DirectoryEntry *Dir = File->getDir();
  llvm::SmallVector<const DirectoryEntry *, 2> SkippedDirs;
#ifdef LLVM_ON_UNIX
  // Note: as an egregious but useful hack we use the real path here, because
  // frameworks moving from top-level frameworks to embedded frameworks tend
  // to be symlinked from the top-level location to the embedded location,
  // and we need to resolve lookups as if we had found the embedded location.
  char RealDirName[PATH_MAX];
  StringRef DirName;
  if (realpath(Dir->getName(), RealDirName))
    DirName = RealDirName;
  else
    DirName = Dir->getName();
#else
  StringRef DirName = Dir->getName();
#endif

  // Keep walking up the directory hierarchy, looking for a directory with
  // an umbrella header.
  do {    
    llvm::DenseMap<const DirectoryEntry *, PCModule *>::iterator KnownDir
      = UmbrellaDirs.find(Dir);
    if (KnownDir != UmbrellaDirs.end()) {
      PCModule *Result = KnownDir->second;
      
      // Search up the module stack until we find a module with an umbrella
      // directory.
      PCModule *UmbrellaPCModule = Result;
      while (!UmbrellaPCModule->getUmbrellaDir() && UmbrellaPCModule->Parent)
        UmbrellaPCModule = UmbrellaPCModule->Parent;

      if (UmbrellaPCModule->InferSubmodules) {
        // Infer submodules for each of the directories we found between
        // the directory of the umbrella header and the directory where 
        // the actual header is located.
        bool Explicit = UmbrellaPCModule->InferExplicitSubmodules;
        
        for (unsigned I = SkippedDirs.size(); I != 0; --I) {
          // Find or create the module that corresponds to this directory name.
          SmallString<32> NameBuf;
          StringRef Name = sanitizeFilenameAsIdentifier(
                             llvm::sys::path::stem(SkippedDirs[I-1]->getName()),
                             NameBuf);
          Result = findOrCreatePCModule(Name, Result, /*IsFramework=*/false,
                                      Explicit).first;
          
          // Associate the module and the directory.
          UmbrellaDirs[SkippedDirs[I-1]] = Result;

          // If inferred submodules export everything they import, add a 
          // wildcard to the set of exports.
          if (UmbrellaPCModule->InferExportWildcard && Result->Exports.empty())
            Result->Exports.push_back(PCModule::ExportDecl(0, true));
        }
        
        // Infer a submodule with the same name as this header file.
        SmallString<32> NameBuf;
        StringRef Name = sanitizeFilenameAsIdentifier(
                           llvm::sys::path::stem(File->getName()), NameBuf);
        Result = findOrCreatePCModule(Name, Result, /*IsFramework=*/false,
                                    Explicit).first;
        Result->TopHeaders.insert(File);
        
        // If inferred submodules export everything they import, add a 
        // wildcard to the set of exports.
        if (UmbrellaPCModule->InferExportWildcard && Result->Exports.empty())
          Result->Exports.push_back(PCModule::ExportDecl(0, true));
      } else {
        // Record each of the directories we stepped through as being part of
        // the module we found, since the umbrella header covers them all.
        for (unsigned I = 0, N = SkippedDirs.size(); I != N; ++I)
          UmbrellaDirs[SkippedDirs[I]] = Result;
      }
      
      Headers[File] = KnownHeader(Result, /*Excluded=*/false);

      // If a header corresponds to an unavailable module, don't report
      // that it maps to anything.
      if (!Result->isAvailable())
        return 0;

      return Result;
    }
    
    SkippedDirs.push_back(Dir);
    
    // Retrieve our parent path.
    DirName = llvm::sys::path::parent_path(DirName);
    if (DirName.empty())
      break;
    
    // Resolve the parent path to a directory entry.
    Dir = SourceMgr->getFileManager().getDirectory(DirName);
  } while (Dir);
  
  return 0;
}

bool PCModuleMap::isHeaderInUnavailablePCModule(const FileEntry *Header) {
  HeadersMap::iterator Known = Headers.find(Header);
  if (Known != Headers.end())
    return !Known->second.isAvailable();
  
  const DirectoryEntry *Dir = Header->getDir();
  llvm::SmallVector<const DirectoryEntry *, 2> SkippedDirs;
  StringRef DirName = Dir->getName();

  // Keep walking up the directory hierarchy, looking for a directory with
  // an umbrella header.
  do {    
    llvm::DenseMap<const DirectoryEntry *, PCModule *>::iterator KnownDir
      = UmbrellaDirs.find(Dir);
    if (KnownDir != UmbrellaDirs.end()) {
      PCModule *Found = KnownDir->second;
      if (!Found->isAvailable())
        return true;

      // Search up the module stack until we find a module with an umbrella
      // directory.
      PCModule *UmbrellaPCModule = Found;
      while (!UmbrellaPCModule->getUmbrellaDir() && UmbrellaPCModule->Parent)
        UmbrellaPCModule = UmbrellaPCModule->Parent;

      if (UmbrellaPCModule->InferSubmodules) {
        for (unsigned I = SkippedDirs.size(); I != 0; --I) {
          // Find or create the module that corresponds to this directory name.
          SmallString<32> NameBuf;
          StringRef Name = sanitizeFilenameAsIdentifier(
                             llvm::sys::path::stem(SkippedDirs[I-1]->getName()),
                             NameBuf);
          Found = lookupPCModuleQualified(Name, Found);
          if (!Found)
            return false;
          if (!Found->isAvailable())
            return true;
        }
        
        // Infer a submodule with the same name as this header file.
        SmallString<32> NameBuf;
        StringRef Name = sanitizeFilenameAsIdentifier(
                           llvm::sys::path::stem(Header->getName()),
                           NameBuf);
        Found = lookupPCModuleQualified(Name, Found);
        if (!Found)
          return false;
      }

      return !Found->isAvailable();
    }
    
    SkippedDirs.push_back(Dir);
    
    // Retrieve our parent path.
    DirName = llvm::sys::path::parent_path(DirName);
    if (DirName.empty())
      break;
    
    // Resolve the parent path to a directory entry.
    Dir = SourceMgr->getFileManager().getDirectory(DirName);
  } while (Dir);
  
  return false;
}

PCModule *PCModuleMap::findPCModule(StringRef Name) {
  llvm::StringMap<PCModule *>::iterator Known = PCModules.find(Name);
  if (Known != PCModules.end())
    return Known->getValue();
  
  return 0;
}

PCModule *PCModuleMap::lookupPCModuleUnqualified(StringRef Name, PCModule *Context) {
  for(; Context; Context = Context->Parent) {
    if (PCModule *Sub = lookupPCModuleQualified(Name, Context))
      return Sub;
  }
  
  return findPCModule(Name);
}

PCModule *PCModuleMap::lookupPCModuleQualified(StringRef Name, PCModule *Context) {
  if (!Context)
    return findPCModule(Name);
  
  return Context->findSubmodule(Name);
}

std::pair<PCModule *, bool> 
PCModuleMap::findOrCreatePCModule(StringRef Name, PCModule *Parent, bool IsFramework,
                              bool IsExplicit) {
  // Try to find an existing module with this name.
  if (PCModule *Sub = lookupPCModuleQualified(Name, Parent))
    return std::make_pair(Sub, false);
  
  // Create a new module with this name.
  PCModule *Result = new PCModule(Name, SourceLocation(), Parent, IsFramework, 
                              IsExplicit);
  if (!Parent)
    PCModules[Name] = Result;
  return std::make_pair(Result, true);
}

bool PCModuleMap::canInferFrameworkPCModule(const DirectoryEntry *ParentDir,
                                        StringRef Name, bool &IsSystem) {
  // Check whether we have already looked into the parent directory
  // for a module map.
  llvm::DenseMap<const DirectoryEntry *, InferredDirectory>::iterator
    inferred = InferredDirectories.find(ParentDir);
  if (inferred == InferredDirectories.end())
    return false;

  if (!inferred->second.InferPCModules)
    return false;

  // We're allowed to infer for this directory, but make sure it's okay
  // to infer this particular module.
  bool canInfer = std::find(inferred->second.ExcludedPCModules.begin(),
                            inferred->second.ExcludedPCModules.end(),
                            Name) == inferred->second.ExcludedPCModules.end();

  if (canInfer && inferred->second.InferSystemPCModules)
    IsSystem = true;

  return canInfer;
}

PCModule *
PCModuleMap::inferFrameworkPCModule(StringRef PCModuleName,
                                const DirectoryEntry *FrameworkDir,
                                bool IsSystem,
                                PCModule *Parent) {
  // Check whether we've already found this module.
  if (PCModule *Mod = lookupPCModuleQualified(PCModuleName, Parent))
    return Mod;
  
  FileManager &FileMgr = SourceMgr->getFileManager();

  // If the framework has a parent path from which we're allowed to infer
  // a framework module, do so.
  if (!Parent) {
    bool canInfer = false;
    if (llvm::sys::path::has_parent_path(FrameworkDir->getName())) {
      // Figure out the parent path.
      StringRef Parent = llvm::sys::path::parent_path(FrameworkDir->getName());
      if (const DirectoryEntry *ParentDir = FileMgr.getDirectory(Parent)) {
        // Check whether we have already looked into the parent directory
        // for a module map.
        llvm::DenseMap<const DirectoryEntry *, InferredDirectory>::iterator
          inferred = InferredDirectories.find(ParentDir);
        if (inferred == InferredDirectories.end()) {
          // We haven't looked here before. Load a module map, if there is
          // one.
          SmallString<128> ModMapPath = Parent;
          llvm::sys::path::append(ModMapPath, "module.map");
          if (const FileEntry *ModMapFile = FileMgr.getFile(ModMapPath)) {
            parsePCModuleMapFile(ModMapFile);
            inferred = InferredDirectories.find(ParentDir);
          }

          if (inferred == InferredDirectories.end())
            inferred = InferredDirectories.insert(
                         std::make_pair(ParentDir, InferredDirectory())).first;
        }

        if (inferred->second.InferPCModules) {
          // We're allowed to infer for this directory, but make sure it's okay
          // to infer this particular module.
          StringRef Name = llvm::sys::path::filename(FrameworkDir->getName());
          canInfer = std::find(inferred->second.ExcludedPCModules.begin(),
                               inferred->second.ExcludedPCModules.end(),
                               Name) == inferred->second.ExcludedPCModules.end();

          if (inferred->second.InferSystemPCModules)
            IsSystem = true;
        }
      }
    }

    // If we're not allowed to infer a framework module, don't.
    if (!canInfer)
      return 0;
  }


  // Look for an umbrella header.
  SmallString<128> UmbrellaName = StringRef(FrameworkDir->getName());
  llvm::sys::path::append(UmbrellaName, "Headers");
  llvm::sys::path::append(UmbrellaName, PCModuleName + ".h");
  const FileEntry *UmbrellaHeader = FileMgr.getFile(UmbrellaName);
  
  // FIXME: If there's no umbrella header, we could probably scan the
  // framework to load *everything*. But, it's not clear that this is a good
  // idea.
  if (!UmbrellaHeader)
    return 0;
  
  PCModule *Result = new PCModule(PCModuleName, SourceLocation(), Parent,
                              /*IsFramework=*/true, /*IsExplicit=*/false);
  if (IsSystem)
    Result->IsSystem = IsSystem;
  
  if (!Parent)
    PCModules[PCModuleName] = Result;
  
  // umbrella header "umbrella-header-name"
  Result->Umbrella = UmbrellaHeader;
  Headers[UmbrellaHeader] = KnownHeader(Result, /*Excluded=*/false);
  UmbrellaDirs[UmbrellaHeader->getDir()] = Result;
  
  // export *
  Result->Exports.push_back(PCModule::ExportDecl(0, true));
  
  // module * { export * }
  Result->InferSubmodules = true;
  Result->InferExportWildcard = true;
  
  // Look for subframeworks.
  llvm::error_code EC;
  SmallString<128> SubframeworksDirName
    = StringRef(FrameworkDir->getName());
  llvm::sys::path::append(SubframeworksDirName, "Frameworks");
  SmallString<128> SubframeworksDirNameNative;
  llvm::sys::path::native(SubframeworksDirName.str(),
                          SubframeworksDirNameNative);
  for (llvm::sys::fs::directory_iterator 
         Dir(SubframeworksDirNameNative.str(), EC), DirEnd;
       Dir != DirEnd && !EC; Dir.increment(EC)) {
    if (!StringRef(Dir->path()).endswith(".framework"))
      continue;

    if (const DirectoryEntry *SubframeworkDir
          = FileMgr.getDirectory(Dir->path())) {
      // Note: as an egregious but useful hack, we use the real path here and
      // check whether it is actually a subdirectory of the parent directory.
      // This will not be the case if the 'subframework' is actually a symlink
      // out to a top-level framework.
#ifdef LLVM_ON_UNIX
      char RealSubframeworkDirName[PATH_MAX];
      if (realpath(Dir->path().c_str(), RealSubframeworkDirName)) {
        StringRef SubframeworkDirName = RealSubframeworkDirName;

        bool FoundParent = false;
        do {
          // Get the parent directory name.
          SubframeworkDirName
            = llvm::sys::path::parent_path(SubframeworkDirName);
          if (SubframeworkDirName.empty())
            break;

          if (FileMgr.getDirectory(SubframeworkDirName) == FrameworkDir) {
            FoundParent = true;
            break;
          }
        } while (true);

        if (!FoundParent)
          continue;
      }
#endif

      // FIXME: Do we want to warn about subframeworks without umbrella headers?
      SmallString<32> NameBuf;
      inferFrameworkPCModule(sanitizeFilenameAsIdentifier(
                             llvm::sys::path::stem(Dir->path()), NameBuf),
                           SubframeworkDir, IsSystem, Result);
    }
  }

  return Result;
}

void PCModuleMap::setUmbrellaHeader(PCModule *Mod, const FileEntry *UmbrellaHeader){
  Headers[UmbrellaHeader] = KnownHeader(Mod, /*Excluded=*/false);
  Mod->Umbrella = UmbrellaHeader;
  UmbrellaDirs[UmbrellaHeader->getDir()] = Mod;
}

void PCModuleMap::setUmbrellaDir(PCModule *Mod, const DirectoryEntry *UmbrellaDir) {
  Mod->Umbrella = UmbrellaDir;
  UmbrellaDirs[UmbrellaDir] = Mod;
}

void PCModuleMap::addHeader(PCModule *Mod, const FileEntry *Header,
                          bool Excluded) {
  if (Excluded)
    Mod->ExcludedHeaders.push_back(Header);
  else
    Mod->Headers.push_back(Header);
  Headers[Header] = KnownHeader(Mod, Excluded);
}

const FileEntry *
PCModuleMap::getContainingPCModuleMapFile(PCModule *PCModule) {
  if (PCModule->DefinitionLoc.isInvalid() || !SourceMgr)
    return 0;

  return SourceMgr->getFileEntryForID(
           SourceMgr->getFileID(PCModule->DefinitionLoc));
}

void PCModuleMap::dump() {
  llvm::errs() << "PCModules:";
  for (llvm::StringMap<PCModule *>::iterator M = PCModules.begin(), 
                                        MEnd = PCModules.end(); 
       M != MEnd; ++M)
    M->getValue()->print(llvm::errs(), 2);
  
  llvm::errs() << "Headers:";
  for (HeadersMap::iterator H = Headers.begin(), HEnd = Headers.end();
       H != HEnd; ++H) {
    llvm::errs() << "  \"" << H->first->getName() << "\" -> " 
                 << H->second.getPCModule()->getFullPCModuleName() << "\n";
  }
}

bool PCModuleMap::resolveExports(PCModule *Mod, bool Complain) {
  bool HadError = false;
  for (unsigned I = 0, N = Mod->UnresolvedExports.size(); I != N; ++I) {
    PCModule::ExportDecl Export = resolveExport(Mod, Mod->UnresolvedExports[I], 
                                              Complain);
    if (Export.getPointer() || Export.getInt())
      Mod->Exports.push_back(Export);
    else
      HadError = true;
  }
  Mod->UnresolvedExports.clear();
  return HadError;
}

PCModule *PCModuleMap::inferPCModuleFromLocation(FullSourceLoc Loc) {
  if (Loc.isInvalid())
    return 0;
  
  // Use the expansion location to determine which module we're in.
  FullSourceLoc ExpansionLoc = Loc.getExpansionLoc();
  if (!ExpansionLoc.isFileID())
    return 0;  
  
  
  const SourceManager &SrcMgr = Loc.getManager();
  FileID ExpansionFileID = ExpansionLoc.getFileID();
  
  while (const FileEntry *ExpansionFile
           = SrcMgr.getFileEntryForID(ExpansionFileID)) {
    // Find the module that owns this header (if any).
    if (PCModule *Mod = findPCModuleForHeader(ExpansionFile))
      return Mod;
    
    // No module owns this header, so look up the inclusion chain to see if
    // any included header has an associated module.
    SourceLocation IncludeLoc = SrcMgr.getIncludeLoc(ExpansionFileID);
    if (IncludeLoc.isInvalid())
      return 0;
    
    ExpansionFileID = SrcMgr.getFileID(IncludeLoc);
  }
  
  return 0;
}

//----------------------------------------------------------------------------//
// PCModule map file parser
//----------------------------------------------------------------------------//

namespace lfort {
  /// \brief A token in a module map file.
  struct MMToken {
    enum TokenKind {
      Comma,
      EndOfFile,
      HeaderKeyword,
      Identifier,
      ExcludeKeyword,
      ExplicitKeyword,
      ExportKeyword,
      FrameworkKeyword,
      ModuleKeyword,
      Period,
      UmbrellaKeyword,
      RequiresKeyword,
      Star,
      StringLiteral,
      LBrace,
      RBrace,
      LSquare,
      RSquare
    } Kind;
    
    unsigned Location;
    unsigned StringLength;
    const char *StringData;
    
    void clear() {
      Kind = EndOfFile;
      Location = 0;
      StringLength = 0;
      StringData = 0;
    }
    
    bool is(TokenKind K) const { return Kind == K; }
    
    SourceLocation getLocation() const {
      return SourceLocation::getFromRawEncoding(Location);
    }
    
    StringRef getString() const {
      return StringRef(StringData, StringLength);
    }
  };

  /// \brief The set of attributes that can be attached to a module.
  struct Attributes {
    Attributes() : IsSystem() { }

    /// \brief Whether this is a system module.
    unsigned IsSystem : 1;
  };
  

  class PCModuleMapParser {
    Lexer &L;
    SourceManager &SourceMgr;

    /// \brief Default target information, used only for string literal
    /// parsing.
    const TargetInfo *Target;

    DiagnosticsEngine &Diags;
    PCModuleMap &Map;
    
    /// \brief The directory that this module map resides in.
    const DirectoryEntry *Directory;

    /// \brief The directory containing LFort-supplied headers.
    const DirectoryEntry *BuiltinIncludeDir;

    /// \brief Whether an error occurred.
    bool HadError;
        
    /// \brief Stores string data for the various string literals referenced
    /// during parsing.
    llvm::BumpPtrAllocator StringData;
    
    /// \brief The current token.
    MMToken Tok;
    
    /// \brief The active module.
    PCModule *ActivePCModule;
    
    /// \brief Consume the current token and return its location.
    SourceLocation consumeToken();
    
    /// \brief Skip tokens until we reach the a token with the given kind
    /// (or the end of the file).
    void skipUntil(MMToken::TokenKind K);

    typedef llvm::SmallVector<std::pair<std::string, SourceLocation>, 2>
      PCModuleId;
    bool parsePCModuleId(PCModuleId &Id);
    void parsePCModuleDecl();
    void parseRequiresDecl();
    void parseHeaderDecl(SourceLocation UmbrellaLoc, SourceLocation ExcludeLoc);
    void parseUmbrellaDirDecl(SourceLocation UmbrellaLoc);
    void parseExportDecl();
    void parseInferredPCModuleDecl(bool Framework, bool Explicit);
    bool parseOptionalAttributes(Attributes &Attrs);

    const DirectoryEntry *getOverriddenHeaderSearchDir();
    
  public:
    explicit PCModuleMapParser(Lexer &L, SourceManager &SourceMgr, 
                             const TargetInfo *Target,
                             DiagnosticsEngine &Diags,
                             PCModuleMap &Map,
                             const DirectoryEntry *Directory,
                             const DirectoryEntry *BuiltinIncludeDir)
      : L(L), SourceMgr(SourceMgr), Target(Target), Diags(Diags), Map(Map), 
        Directory(Directory), BuiltinIncludeDir(BuiltinIncludeDir), 
        HadError(false), ActivePCModule(0)
    {
      Tok.clear();
      consumeToken();
    }
    
    bool parsePCModuleMapFile();
  };
}

SourceLocation PCModuleMapParser::consumeToken() {
retry:
  SourceLocation Result = Tok.getLocation();
  Tok.clear();
  
  Token LToken;
  L.LexFromRawLexer(LToken);
  Tok.Location = LToken.getLocation().getRawEncoding();
  switch (LToken.getKind()) {
  case tok::raw_identifier:
    Tok.StringData = LToken.getRawIdentifierData();
    Tok.StringLength = LToken.getLength();
    Tok.Kind = llvm::StringSwitch<MMToken::TokenKind>(Tok.getString())
                 .Case("header", MMToken::HeaderKeyword)
                 .Case("exclude", MMToken::ExcludeKeyword)
                 .Case("explicit", MMToken::ExplicitKeyword)
                 .Case("export", MMToken::ExportKeyword)
                 .Case("framework", MMToken::FrameworkKeyword)
                 .Case("module", MMToken::ModuleKeyword)
                 .Case("requires", MMToken::RequiresKeyword)
                 .Case("umbrella", MMToken::UmbrellaKeyword)
                 .Default(MMToken::Identifier);
    break;

  case tok::comma:
    Tok.Kind = MMToken::Comma;
    break;

  case tok::eof:
    Tok.Kind = MMToken::EndOfFile;
    break;
      
  case tok::l_brace:
    Tok.Kind = MMToken::LBrace;
    break;

  case tok::l_square:
    Tok.Kind = MMToken::LSquare;
    break;
      
  case tok::period:
    Tok.Kind = MMToken::Period;
    break;
      
  case tok::r_brace:
    Tok.Kind = MMToken::RBrace;
    break;
      
  case tok::r_square:
    Tok.Kind = MMToken::RSquare;
    break;
      
  case tok::star:
    Tok.Kind = MMToken::Star;
    break;
      
  case tok::string_literal: {
    if (LToken.hasUDSuffix()) {
      Diags.Report(LToken.getLocation(), diag::err_invalid_string_udl);
      HadError = true;
      goto retry;
    }

    // Parse the string literal.
    LangOptions LangOpts;
    StringLiteralParser StringLiteral(&LToken, 1, SourceMgr, LangOpts, *Target);
    if (StringLiteral.hadError)
      goto retry;
    
    // Copy the string literal into our string data allocator.
    unsigned Length = StringLiteral.GetStringLength();
    char *Saved = StringData.Allocate<char>(Length + 1);
    memcpy(Saved, StringLiteral.GetString().data(), Length);
    Saved[Length] = 0;
    
    // Form the token.
    Tok.Kind = MMToken::StringLiteral;
    Tok.StringData = Saved;
    Tok.StringLength = Length;
    break;
  }
      
  case tok::comment:
    goto retry;
      
  default:
    Diags.Report(LToken.getLocation(), diag::err_mmap_unknown_token);
    HadError = true;
    goto retry;
  }
  
  return Result;
}

void PCModuleMapParser::skipUntil(MMToken::TokenKind K) {
  unsigned braceDepth = 0;
  unsigned squareDepth = 0;
  do {
    switch (Tok.Kind) {
    case MMToken::EndOfFile:
      return;

    case MMToken::LBrace:
      if (Tok.is(K) && braceDepth == 0 && squareDepth == 0)
        return;
        
      ++braceDepth;
      break;

    case MMToken::LSquare:
      if (Tok.is(K) && braceDepth == 0 && squareDepth == 0)
        return;
      
      ++squareDepth;
      break;

    case MMToken::RBrace:
      if (braceDepth > 0)
        --braceDepth;
      else if (Tok.is(K))
        return;
      break;

    case MMToken::RSquare:
      if (squareDepth > 0)
        --squareDepth;
      else if (Tok.is(K))
        return;
      break;

    default:
      if (braceDepth == 0 && squareDepth == 0 && Tok.is(K))
        return;
      break;
    }
    
   consumeToken();
  } while (true);
}

/// \brief Parse a module-id.
///
///   module-id:
///     identifier
///     identifier '.' module-id
///
/// \returns true if an error occurred, false otherwise.
bool PCModuleMapParser::parsePCModuleId(PCModuleId &Id) {
  Id.clear();
  do {
    if (Tok.is(MMToken::Identifier)) {
      Id.push_back(std::make_pair(Tok.getString(), Tok.getLocation()));
      consumeToken();
    } else {
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_module_name);
      return true;
    }
    
    if (!Tok.is(MMToken::Period))
      break;
    
    consumeToken();
  } while (true);
  
  return false;
}

namespace {
  /// \brief Enumerates the known attributes.
  enum AttributeKind {
    /// \brief An unknown attribute.
    AT_unknown,
    /// \brief The 'system' attribute.
    AT_system
  };
}

/// \brief Parse a module declaration.
///
///   module-declaration:
///     'explicit'[opt] 'framework'[opt] 'module' module-id attributes[opt] 
///       { module-member* }
///
///   module-member:
///     requires-declaration
///     header-declaration
///     submodule-declaration
///     export-declaration
///
///   submodule-declaration:
///     module-declaration
///     inferred-submodule-declaration
void PCModuleMapParser::parsePCModuleDecl() {
  assert(Tok.is(MMToken::ExplicitKeyword) || Tok.is(MMToken::ModuleKeyword) ||
         Tok.is(MMToken::FrameworkKeyword));
  // Parse 'explicit' or 'framework' keyword, if present.
  SourceLocation ExplicitLoc;
  bool Explicit = false;
  bool Framework = false;

  // Parse 'explicit' keyword, if present.
  if (Tok.is(MMToken::ExplicitKeyword)) {
    ExplicitLoc = consumeToken();
    Explicit = true;
  }

  // Parse 'framework' keyword, if present.
  if (Tok.is(MMToken::FrameworkKeyword)) {
    consumeToken();
    Framework = true;
  } 
  
  // Parse 'module' keyword.
  if (!Tok.is(MMToken::ModuleKeyword)) {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_module);
    consumeToken();
    HadError = true;
    return;
  }
  consumeToken(); // 'module' keyword

  // If we have a wildcard for the module name, this is an inferred submodule.
  // Parse it. 
  if (Tok.is(MMToken::Star))
    return parseInferredPCModuleDecl(Framework, Explicit);
  
  // Parse the module name.
  PCModuleId Id;
  if (parsePCModuleId(Id)) {
    HadError = true;
    return;
  }

  if (ActivePCModule) {
    if (Id.size() > 1) {
      Diags.Report(Id.front().second, diag::err_mmap_nested_submodule_id)
        << SourceRange(Id.front().second, Id.back().second);
      
      HadError = true;
      return;
    }
  } else if (Id.size() == 1 && Explicit) {
    // Top-level modules can't be explicit.
    Diags.Report(ExplicitLoc, diag::err_mmap_explicit_top_level);
    Explicit = false;
    ExplicitLoc = SourceLocation();
    HadError = true;
  }
  
  PCModule *PreviousActivePCModule = ActivePCModule;  
  if (Id.size() > 1) {
    // This module map defines a submodule. Go find the module of which it
    // is a submodule.
    ActivePCModule = 0;
    for (unsigned I = 0, N = Id.size() - 1; I != N; ++I) {
      if (PCModule *Next = Map.lookupPCModuleQualified(Id[I].first, ActivePCModule)) {
        ActivePCModule = Next;
        continue;
      }
      
      if (ActivePCModule) {
        Diags.Report(Id[I].second, diag::err_mmap_missing_module_qualified)
          << Id[I].first << ActivePCModule->getTopLevelPCModule();
      } else {
        Diags.Report(Id[I].second, diag::err_mmap_expected_module_name);
      }
      HadError = true;
      return;
    }
  } 
  
  StringRef PCModuleName = Id.back().first;
  SourceLocation PCModuleNameLoc = Id.back().second;
  
  // Parse the optional attribute list.
  Attributes Attrs;
  parseOptionalAttributes(Attrs);
  
  // Parse the opening brace.
  if (!Tok.is(MMToken::LBrace)) {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_lbrace)
      << PCModuleName;
    HadError = true;
    return;
  }  
  SourceLocation LBraceLoc = consumeToken();
  
  // Determine whether this (sub)module has already been defined.
  if (PCModule *Existing = Map.lookupPCModuleQualified(PCModuleName, ActivePCModule)) {
    if (Existing->DefinitionLoc.isInvalid() && !ActivePCModule) {
      // Skip the module definition.
      skipUntil(MMToken::RBrace);
      if (Tok.is(MMToken::RBrace))
        consumeToken();
      else {
        Diags.Report(Tok.getLocation(), diag::err_mmap_expected_rbrace);
        Diags.Report(LBraceLoc, diag::note_mmap_lbrace_match);
        HadError = true;        
      }
      return;
    }
    
    Diags.Report(PCModuleNameLoc, diag::err_mmap_module_redefinition)
      << PCModuleName;
    Diags.Report(Existing->DefinitionLoc, diag::note_mmap_prev_definition);
    
    // Skip the module definition.
    skipUntil(MMToken::RBrace);
    if (Tok.is(MMToken::RBrace))
      consumeToken();
    
    HadError = true;
    return;
  }

  // Start defining this module.
  ActivePCModule = Map.findOrCreatePCModule(PCModuleName, ActivePCModule, Framework,
                                        Explicit).first;
  ActivePCModule->DefinitionLoc = PCModuleNameLoc;
  if (Attrs.IsSystem)
    ActivePCModule->IsSystem = true;
  
  bool Done = false;
  do {
    switch (Tok.Kind) {
    case MMToken::EndOfFile:
    case MMToken::RBrace:
      Done = true;
      break;
        
    case MMToken::ExplicitKeyword:
    case MMToken::FrameworkKeyword:
    case MMToken::ModuleKeyword:
      parsePCModuleDecl();
      break;
        
    case MMToken::ExportKeyword:
      parseExportDecl();
      break;
        
    case MMToken::RequiresKeyword:
      parseRequiresDecl();
      break;

    case MMToken::UmbrellaKeyword: {
      SourceLocation UmbrellaLoc = consumeToken();
      if (Tok.is(MMToken::HeaderKeyword))
        parseHeaderDecl(UmbrellaLoc, SourceLocation());
      else
        parseUmbrellaDirDecl(UmbrellaLoc);
      break;
    }
        
    case MMToken::ExcludeKeyword: {
      SourceLocation ExcludeLoc = consumeToken();
      if (Tok.is(MMToken::HeaderKeyword)) {
        parseHeaderDecl(SourceLocation(), ExcludeLoc);
      } else {
        Diags.Report(Tok.getLocation(), diag::err_mmap_expected_header)
          << "exclude";
      }
      break;
    }
      
    case MMToken::HeaderKeyword:
      parseHeaderDecl(SourceLocation(), SourceLocation());
      break;
        
    default:
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_member);
      consumeToken();
      break;        
    }
  } while (!Done);

  if (Tok.is(MMToken::RBrace))
    consumeToken();
  else {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_rbrace);
    Diags.Report(LBraceLoc, diag::note_mmap_lbrace_match);
    HadError = true;
  }

  // We're done parsing this module. Pop back to the previous module.
  ActivePCModule = PreviousActivePCModule;
}

/// \brief Parse a requires declaration.
///
///   requires-declaration:
///     'requires' feature-list
///
///   feature-list:
///     identifier ',' feature-list
///     identifier
void PCModuleMapParser::parseRequiresDecl() {
  assert(Tok.is(MMToken::RequiresKeyword));

  // Parse 'requires' keyword.
  consumeToken();

  // Parse the feature-list.
  do {
    if (!Tok.is(MMToken::Identifier)) {
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_feature);
      HadError = true;
      return;
    }

    // Consume the feature name.
    std::string Feature = Tok.getString();
    consumeToken();

    // Add this feature.
    ActivePCModule->addRequirement(Feature, Map.LangOpts, *Map.Target);

    if (!Tok.is(MMToken::Comma))
      break;

    // Consume the comma.
    consumeToken();
  } while (true);
}

/// \brief Append to \p Paths the set of paths needed to get to the 
/// subframework in which the given module lives.
static void appendSubframeworkPaths(PCModule *Mod,
                                    llvm::SmallVectorImpl<char> &Path) {
  // Collect the framework names from the given module to the top-level module.
  llvm::SmallVector<StringRef, 2> Paths;
  for (; Mod; Mod = Mod->Parent) {
    if (Mod->IsFramework)
      Paths.push_back(Mod->Name);
  }
  
  if (Paths.empty())
    return;
  
  // Add Frameworks/Name.framework for each subframework.
  for (unsigned I = Paths.size() - 1; I != 0; --I) {
    llvm::sys::path::append(Path, "Frameworks");
    llvm::sys::path::append(Path, Paths[I-1] + ".framework");
  }
}

/// \brief Determine whether the given file name is the name of a builtin
/// header, supplied by LFort to replace, override, or augment existing system
/// headers.
static bool isBuiltinHeader(StringRef FileName) {
  return llvm::StringSwitch<bool>(FileName)
      .Case("float.h", true)
      .Case("iso646.h", true)
      .Case("limits.h", true)
      .Case("stdalign.h", true)
      .Case("stdarg.h", true)
      .Case("stdbool.h", true)
      .Case("stddef.h", true)
      .Case("stdint.h", true)
      .Case("tgmath.h", true)
      .Case("unwind.h", true)
      .Default(false);
}

/// \brief Parse a header declaration.
///
///   header-declaration:
///     'umbrella'[opt] 'header' string-literal
///     'exclude'[opt] 'header' string-literal
void PCModuleMapParser::parseHeaderDecl(SourceLocation UmbrellaLoc,
                                      SourceLocation ExcludeLoc) {
  assert(Tok.is(MMToken::HeaderKeyword));
  consumeToken();

  bool Umbrella = UmbrellaLoc.isValid();
  bool Exclude = ExcludeLoc.isValid();
  assert(!(Umbrella && Exclude) && "Cannot have both 'umbrella' and 'exclude'");
  // Parse the header name.
  if (!Tok.is(MMToken::StringLiteral)) {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_header) 
      << "header";
    HadError = true;
    return;
  }
  std::string FileName = Tok.getString();
  SourceLocation FileNameLoc = consumeToken();
  
  // Check whether we already have an umbrella.
  if (Umbrella && ActivePCModule->Umbrella) {
    Diags.Report(FileNameLoc, diag::err_mmap_umbrella_clash)
      << ActivePCModule->getFullPCModuleName();
    HadError = true;
    return;
  }

  // Look for this file.
  const FileEntry *File = 0;
  const FileEntry *BuiltinFile = 0;
  SmallString<128> PathName;
  if (llvm::sys::path::is_absolute(FileName)) {
    PathName = FileName;
    File = SourceMgr.getFileManager().getFile(PathName);
  } else if (const DirectoryEntry *Dir = getOverriddenHeaderSearchDir()) {
    PathName = Dir->getName();
    llvm::sys::path::append(PathName, FileName);
    File = SourceMgr.getFileManager().getFile(PathName);
  } else {
    // Search for the header file within the search directory.
    PathName = Directory->getName();
    unsigned PathLength = PathName.size();
    
    if (ActivePCModule->isPartOfFramework()) {
      appendSubframeworkPaths(ActivePCModule, PathName);
      
      // Check whether this file is in the public headers.
      llvm::sys::path::append(PathName, "Headers");
      llvm::sys::path::append(PathName, FileName);
      File = SourceMgr.getFileManager().getFile(PathName);
      
      if (!File) {
        // Check whether this file is in the private headers.
        PathName.resize(PathLength);
        llvm::sys::path::append(PathName, "PrivateHeaders");
        llvm::sys::path::append(PathName, FileName);
        File = SourceMgr.getFileManager().getFile(PathName);
      }
    } else {
      // Lookup for normal headers.
      llvm::sys::path::append(PathName, FileName);
      File = SourceMgr.getFileManager().getFile(PathName);
      
      // If this is a system module with a top-level header, this header
      // may have a counterpart (or replacement) in the set of headers
      // supplied by LFort. Find that builtin header.
      if (ActivePCModule->IsSystem && !Umbrella && BuiltinIncludeDir &&
          BuiltinIncludeDir != Directory && isBuiltinHeader(FileName)) {
        SmallString<128> BuiltinPathName(BuiltinIncludeDir->getName());
        llvm::sys::path::append(BuiltinPathName, FileName);
        BuiltinFile = SourceMgr.getFileManager().getFile(BuiltinPathName);
        
        // If LFort supplies this header but the underlying system does not,
        // just silently swap in our builtin version. Otherwise, we'll end
        // up adding both (later).
        if (!File && BuiltinFile) {
          File = BuiltinFile;
          BuiltinFile = 0;
        }
      }
    }
  }
  
  // FIXME: We shouldn't be eagerly stat'ing every file named in a module map.
  // Come up with a lazy way to do this.
  if (File) {
    if (PCModuleMap::KnownHeader OwningPCModule = Map.Headers[File]) {
      Diags.Report(FileNameLoc, diag::err_mmap_header_conflict)
        << FileName << OwningPCModule.getPCModule()->getFullPCModuleName();
      HadError = true;
    } else if (Umbrella) {
      const DirectoryEntry *UmbrellaDir = File->getDir();
      if (PCModule *UmbrellaPCModule = Map.UmbrellaDirs[UmbrellaDir]) {
        Diags.Report(UmbrellaLoc, diag::err_mmap_umbrella_clash)
          << UmbrellaPCModule->getFullPCModuleName();
        HadError = true;
      } else {
        // Record this umbrella header.
        Map.setUmbrellaHeader(ActivePCModule, File);
      }
    } else {
      // Record this header.
      Map.addHeader(ActivePCModule, File, Exclude);
      
      // If there is a builtin counterpart to this file, add it now.
      if (BuiltinFile)
        Map.addHeader(ActivePCModule, BuiltinFile, Exclude);
    }
  } else if (!Exclude) {
    // Ignore excluded header files. They're optional anyway.
    
    Diags.Report(FileNameLoc, diag::err_mmap_header_not_found)
      << Umbrella << FileName;
    HadError = true;
  }
}

/// \brief Parse an umbrella directory declaration.
///
///   umbrella-dir-declaration:
///     umbrella string-literal
void PCModuleMapParser::parseUmbrellaDirDecl(SourceLocation UmbrellaLoc) {
  // Parse the directory name.
  if (!Tok.is(MMToken::StringLiteral)) {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_header) 
      << "umbrella";
    HadError = true;
    return;
  }

  std::string DirName = Tok.getString();
  SourceLocation DirNameLoc = consumeToken();
  
  // Check whether we already have an umbrella.
  if (ActivePCModule->Umbrella) {
    Diags.Report(DirNameLoc, diag::err_mmap_umbrella_clash)
      << ActivePCModule->getFullPCModuleName();
    HadError = true;
    return;
  }

  // Look for this file.
  const DirectoryEntry *Dir = 0;
  if (llvm::sys::path::is_absolute(DirName))
    Dir = SourceMgr.getFileManager().getDirectory(DirName);
  else {
    SmallString<128> PathName;
    PathName = Directory->getName();
    llvm::sys::path::append(PathName, DirName);
    Dir = SourceMgr.getFileManager().getDirectory(PathName);
  }
  
  if (!Dir) {
    Diags.Report(DirNameLoc, diag::err_mmap_umbrella_dir_not_found)
      << DirName;
    HadError = true;
    return;
  }
  
  if (PCModule *OwningPCModule = Map.UmbrellaDirs[Dir]) {
    Diags.Report(UmbrellaLoc, diag::err_mmap_umbrella_clash)
      << OwningPCModule->getFullPCModuleName();
    HadError = true;
    return;
  } 
  
  // Record this umbrella directory.
  Map.setUmbrellaDir(ActivePCModule, Dir);
}

/// \brief Parse a module export declaration.
///
///   export-declaration:
///     'export' wildcard-module-id
///
///   wildcard-module-id:
///     identifier
///     '*'
///     identifier '.' wildcard-module-id
void PCModuleMapParser::parseExportDecl() {
  assert(Tok.is(MMToken::ExportKeyword));
  SourceLocation ExportLoc = consumeToken();
  
  // Parse the module-id with an optional wildcard at the end.
  PCModuleId ParsedPCModuleId;
  bool Wildcard = false;
  do {
    if (Tok.is(MMToken::Identifier)) {
      ParsedPCModuleId.push_back(std::make_pair(Tok.getString(), 
                                              Tok.getLocation()));
      consumeToken();
      
      if (Tok.is(MMToken::Period)) {
        consumeToken();
        continue;
      } 
      
      break;
    }
    
    if(Tok.is(MMToken::Star)) {
      Wildcard = true;
      consumeToken();
      break;
    }
    
    Diags.Report(Tok.getLocation(), diag::err_mmap_export_module_id);
    HadError = true;
    return;
  } while (true);
  
  PCModule::UnresolvedExportDecl Unresolved = { 
    ExportLoc, ParsedPCModuleId, Wildcard 
  };
  ActivePCModule->UnresolvedExports.push_back(Unresolved);
}

/// \brief Parse an inferried module declaration (wildcard modules).
///
///   module-declaration:
///     'explicit'[opt] 'framework'[opt] 'module' * attributes[opt]
///       { inferred-module-member* }
///
///   inferred-module-member:
///     'export' '*'
///     'exclude' identifier
void PCModuleMapParser::parseInferredPCModuleDecl(bool Framework, bool Explicit) {
  assert(Tok.is(MMToken::Star));
  SourceLocation StarLoc = consumeToken();
  bool Failed = false;

  // Inferred modules must be submodules.
  if (!ActivePCModule && !Framework) {
    Diags.Report(StarLoc, diag::err_mmap_top_level_inferred_submodule);
    Failed = true;
  }

  if (ActivePCModule) {
    // Inferred modules must have umbrella directories.
    if (!Failed && !ActivePCModule->getUmbrellaDir()) {
      Diags.Report(StarLoc, diag::err_mmap_inferred_no_umbrella);
      Failed = true;
    }
    
    // Check for redefinition of an inferred module.
    if (!Failed && ActivePCModule->InferSubmodules) {
      Diags.Report(StarLoc, diag::err_mmap_inferred_redef);
      if (ActivePCModule->InferredSubmoduleLoc.isValid())
        Diags.Report(ActivePCModule->InferredSubmoduleLoc,
                     diag::note_mmap_prev_definition);
      Failed = true;
    }

    // Check for the 'framework' keyword, which is not permitted here.
    if (Framework) {
      Diags.Report(StarLoc, diag::err_mmap_inferred_framework_submodule);
      Framework = false;
    }
  } else if (Explicit) {
    Diags.Report(StarLoc, diag::err_mmap_explicit_inferred_framework);
    Explicit = false;
  }

  // If there were any problems with this inferred submodule, skip its body.
  if (Failed) {
    if (Tok.is(MMToken::LBrace)) {
      consumeToken();
      skipUntil(MMToken::RBrace);
      if (Tok.is(MMToken::RBrace))
        consumeToken();
    }
    HadError = true;
    return;
  }

  // Parse optional attributes.
  Attributes Attrs;
  parseOptionalAttributes(Attrs);

  if (ActivePCModule) {
    // Note that we have an inferred submodule.
    ActivePCModule->InferSubmodules = true;
    ActivePCModule->InferredSubmoduleLoc = StarLoc;
    ActivePCModule->InferExplicitSubmodules = Explicit;
  } else {
    // We'll be inferring framework modules for this directory.
    Map.InferredDirectories[Directory].InferPCModules = true;
    Map.InferredDirectories[Directory].InferSystemPCModules = Attrs.IsSystem;
  }

  // Parse the opening brace.
  if (!Tok.is(MMToken::LBrace)) {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_lbrace_wildcard);
    HadError = true;
    return;
  }  
  SourceLocation LBraceLoc = consumeToken();

  // Parse the body of the inferred submodule.
  bool Done = false;
  do {
    switch (Tok.Kind) {
    case MMToken::EndOfFile:
    case MMToken::RBrace:
      Done = true;
      break;

    case MMToken::ExcludeKeyword: {
      if (ActivePCModule) {
        Diags.Report(Tok.getLocation(), diag::err_mmap_expected_inferred_member)
          << (ActivePCModule != 0);
        consumeToken();
        break;
      }

      consumeToken();
      if (!Tok.is(MMToken::Identifier)) {
        Diags.Report(Tok.getLocation(), diag::err_mmap_missing_exclude_name);
        break;
      }

      Map.InferredDirectories[Directory].ExcludedPCModules
        .push_back(Tok.getString());
      consumeToken();
      break;
    }

    case MMToken::ExportKeyword:
      if (!ActivePCModule) {
        Diags.Report(Tok.getLocation(), diag::err_mmap_expected_inferred_member)
          << (ActivePCModule != 0);
        consumeToken();
        break;
      }

      consumeToken();
      if (Tok.is(MMToken::Star)) 
        ActivePCModule->InferExportWildcard = true;
      else
        Diags.Report(Tok.getLocation(), 
                     diag::err_mmap_expected_export_wildcard);
      consumeToken();
      break;

    case MMToken::ExplicitKeyword:
    case MMToken::ModuleKeyword:
    case MMToken::HeaderKeyword:
    case MMToken::UmbrellaKeyword:
    default:
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_inferred_member)
          << (ActivePCModule != 0);
      consumeToken();
      break;        
    }
  } while (!Done);
  
  if (Tok.is(MMToken::RBrace))
    consumeToken();
  else {
    Diags.Report(Tok.getLocation(), diag::err_mmap_expected_rbrace);
    Diags.Report(LBraceLoc, diag::note_mmap_lbrace_match);
    HadError = true;
  }
}

/// \brief Parse optional attributes.
///
///   attributes:
///     attribute attributes
///     attribute
///
///   attribute:
///     [ identifier ]
///
/// \param Attrs Will be filled in with the parsed attributes.
///
/// \returns true if an error occurred, false otherwise.
bool PCModuleMapParser::parseOptionalAttributes(Attributes &Attrs) {
  bool HadError = false;
  
  while (Tok.is(MMToken::LSquare)) {
    // Consume the '['.
    SourceLocation LSquareLoc = consumeToken();

    // Check whether we have an attribute name here.
    if (!Tok.is(MMToken::Identifier)) {
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_attribute);
      skipUntil(MMToken::RSquare);
      if (Tok.is(MMToken::RSquare))
        consumeToken();
      HadError = true;
    }

    // Decode the attribute name.
    AttributeKind Attribute
      = llvm::StringSwitch<AttributeKind>(Tok.getString())
          .Case("system", AT_system)
          .Default(AT_unknown);
    switch (Attribute) {
    case AT_unknown:
      Diags.Report(Tok.getLocation(), diag::warn_mmap_unknown_attribute)
        << Tok.getString();
      break;

    case AT_system:
      Attrs.IsSystem = true;
      break;
    }
    consumeToken();

    // Consume the ']'.
    if (!Tok.is(MMToken::RSquare)) {
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_rsquare);
      Diags.Report(LSquareLoc, diag::note_mmap_lsquare_match);
      skipUntil(MMToken::RSquare);
      HadError = true;
    }

    if (Tok.is(MMToken::RSquare))
      consumeToken();
  }

  return HadError;
}

/// \brief If there is a specific header search directory due the presence
/// of an umbrella directory, retrieve that directory. Otherwise, returns null.
const DirectoryEntry *PCModuleMapParser::getOverriddenHeaderSearchDir() {
  for (PCModule *Mod = ActivePCModule; Mod; Mod = Mod->Parent) {
    // If we have an umbrella directory, use that.
    if (Mod->hasUmbrellaDir())
      return Mod->getUmbrellaDir();
    
    // If we have a framework directory, stop looking.
    if (Mod->IsFramework)
      return 0;
  }
  
  return 0;
}

/// \brief Parse a module map file.
///
///   module-map-file:
///     module-declaration*
bool PCModuleMapParser::parsePCModuleMapFile() {
  do {
    switch (Tok.Kind) {
    case MMToken::EndOfFile:
      return HadError;
      
    case MMToken::ExplicitKeyword:
    case MMToken::ModuleKeyword:
    case MMToken::FrameworkKeyword:
      parsePCModuleDecl();
      break;
      
    case MMToken::Comma:
    case MMToken::ExcludeKeyword:
    case MMToken::ExportKeyword:
    case MMToken::HeaderKeyword:
    case MMToken::Identifier:
    case MMToken::LBrace:
    case MMToken::LSquare:
    case MMToken::Period:
    case MMToken::RBrace:
    case MMToken::RSquare:
    case MMToken::RequiresKeyword:
    case MMToken::Star:
    case MMToken::StringLiteral:
    case MMToken::UmbrellaKeyword:
      Diags.Report(Tok.getLocation(), diag::err_mmap_expected_module);
      HadError = true;
      consumeToken();
      break;
    }
  } while (true);
}

bool PCModuleMap::parsePCModuleMapFile(const FileEntry *File) {
  assert(Target != 0 && "Missing target information");
  FileID ID = SourceMgr->createFileID(File, SourceLocation(), SrcMgr::C_User);
  const llvm::MemoryBuffer *Buffer = SourceMgr->getBuffer(ID);
  if (!Buffer)
    return true;
  
  // Parse this module map file.
  Lexer L(ID, SourceMgr->getBuffer(ID), *SourceMgr, MMapLangOpts);
  Diags->getClient()->BeginSourceFile(MMapLangOpts);
  PCModuleMapParser Parser(L, *SourceMgr, Target, *Diags, *this, File->getDir(),
                         BuiltinIncludeDir);
  bool Result = Parser.parsePCModuleMapFile();
  Diags->getClient()->EndSourceFile();
  
  return Result;
}
