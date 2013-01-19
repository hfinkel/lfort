//===--- PCModule.cpp - Describe a module -----------------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the PCModule class, which describes a module in the source
// code.
//
//===----------------------------------------------------------------------===//
#include "lfort/Basic/PCModule.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Basic/LangOptions.h"
#include "lfort/Basic/TargetInfo.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/StringSwitch.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace lfort;

PCModule::PCModule(StringRef Name, SourceLocation DefinitionLoc, PCModule *Parent, 
               bool IsFramework, bool IsExplicit)
  : Name(Name), DefinitionLoc(DefinitionLoc), Parent(Parent),
    Umbrella(), ASTFile(0), IsAvailable(true), IsFromPCModuleFile(false),
    IsFramework(IsFramework), IsExplicit(IsExplicit), IsSystem(false),
    InferSubmodules(false), InferExplicitSubmodules(false), 
    InferExportWildcard(false), NameVisibility(Hidden) 
{ 
  if (Parent) {
    if (!Parent->isAvailable())
      IsAvailable = false;
    if (Parent->IsSystem)
      IsSystem = true;
    
    Parent->SubPCModuleIndex[Name] = Parent->SubPCModules.size();
    Parent->SubPCModules.push_back(this);
  }
}

PCModule::~PCModule() {
  for (submodule_iterator I = submodule_begin(), IEnd = submodule_end();
       I != IEnd; ++I) {
    delete *I;
  }
  
}

/// \brief Determine whether a translation unit built using the current
/// language options has the given feature.
static bool hasFeature(StringRef Feature, const LangOptions &LangOpts,
                       const TargetInfo &Target) {
  return llvm::StringSwitch<bool>(Feature)
           .Case("altivec", LangOpts.AltiVec)
           .Case("blocks", LangOpts.Blocks)
           .Case("cplusplus", LangOpts.CPlusPlus)
           .Case("cplusplus11", LangOpts.F90)
           .Case("objc", LangOpts.ObjC1)
           .Case("objc_arc", LangOpts.ObjCAutoRefCount)
           .Case("opencl", LangOpts.OpenCL)
           .Case("tls", Target.isTLSSupported())
           .Default(Target.hasFeature(Feature));
}

bool 
PCModule::isAvailable(const LangOptions &LangOpts, const TargetInfo &Target,
                    StringRef &Feature) const {
  if (IsAvailable)
    return true;

  for (const PCModule *Current = this; Current; Current = Current->Parent) {
    for (unsigned I = 0, N = Current->Requires.size(); I != N; ++I) {
      if (!hasFeature(Current->Requires[I], LangOpts, Target)) {
        Feature = Current->Requires[I];
        return false;
      }
    }
  }

  llvm_unreachable("could not find a reason why module is unavailable");
}

bool PCModule::isSubPCModuleOf(PCModule *Other) const {
  const PCModule *This = this;
  do {
    if (This == Other)
      return true;
    
    This = This->Parent;
  } while (This);
  
  return false;
}

const PCModule *PCModule::getTopLevelPCModule() const {
  const PCModule *Result = this;
  while (Result->Parent)
    Result = Result->Parent;
  
  return Result;
}

std::string PCModule::getFullPCModuleName() const {
  llvm::SmallVector<StringRef, 2> Names;
  
  // Build up the set of module names (from innermost to outermost).
  for (const PCModule *M = this; M; M = M->Parent)
    Names.push_back(M->Name);
  
  std::string Result;
  for (llvm::SmallVector<StringRef, 2>::reverse_iterator I = Names.rbegin(),
                                                      IEnd = Names.rend(); 
       I != IEnd; ++I) {
    if (!Result.empty())
      Result += '.';
    
    Result += *I;
  }
  
  return Result;
}

const DirectoryEntry *PCModule::getUmbrellaDir() const {
  if (const FileEntry *Header = getUmbrellaHeader())
    return Header->getDir();
  
  return Umbrella.dyn_cast<const DirectoryEntry *>();
}

void PCModule::addRequirement(StringRef Feature, const LangOptions &LangOpts,
                            const TargetInfo &Target) {
  Requires.push_back(Feature);

  // If this feature is currently available, we're done.
  if (hasFeature(Feature, LangOpts, Target))
    return;

  if (!IsAvailable)
    return;

  llvm::SmallVector<PCModule *, 2> Stack;
  Stack.push_back(this);
  while (!Stack.empty()) {
    PCModule *Current = Stack.back();
    Stack.pop_back();

    if (!Current->IsAvailable)
      continue;

    Current->IsAvailable = false;
    for (submodule_iterator Sub = Current->submodule_begin(),
                         SubEnd = Current->submodule_end();
         Sub != SubEnd; ++Sub) {
      if ((*Sub)->IsAvailable)
        Stack.push_back(*Sub);
    }
  }
}

PCModule *PCModule::findSubmodule(StringRef Name) const {
  llvm::StringMap<unsigned>::const_iterator Pos = SubPCModuleIndex.find(Name);
  if (Pos == SubPCModuleIndex.end())
    return 0;
  
  return SubPCModules[Pos->getValue()];
}

static void printPCModuleId(llvm::raw_ostream &OS, const PCModuleId &Id) {
  for (unsigned I = 0, N = Id.size(); I != N; ++I) {
    if (I)
      OS << ".";
    OS << Id[I].first;
  }
}

void PCModule::print(llvm::raw_ostream &OS, unsigned Indent) const {
  OS.indent(Indent);
  if (IsFramework)
    OS << "framework ";
  if (IsExplicit)
    OS << "explicit ";
  OS << "module " << Name;

  if (IsSystem) {
    OS.indent(Indent + 2);
    OS << " [system]";
  }

  OS << " {\n";
  
  if (!Requires.empty()) {
    OS.indent(Indent + 2);
    OS << "requires ";
    for (unsigned I = 0, N = Requires.size(); I != N; ++I) {
      if (I)
        OS << ", ";
      OS << Requires[I];
    }
    OS << "\n";
  }
  
  if (const FileEntry *UmbrellaHeader = getUmbrellaHeader()) {
    OS.indent(Indent + 2);
    OS << "umbrella header \"";
    OS.write_escaped(UmbrellaHeader->getName());
    OS << "\"\n";
  } else if (const DirectoryEntry *UmbrellaDir = getUmbrellaDir()) {
    OS.indent(Indent + 2);
    OS << "umbrella \"";
    OS.write_escaped(UmbrellaDir->getName());
    OS << "\"\n";    
  }
  
  for (unsigned I = 0, N = Headers.size(); I != N; ++I) {
    OS.indent(Indent + 2);
    OS << "header \"";
    OS.write_escaped(Headers[I]->getName());
    OS << "\"\n";
  }

  for (unsigned I = 0, N = ExcludedHeaders.size(); I != N; ++I) {
    OS.indent(Indent + 2);
    OS << "exclude header \"";
    OS.write_escaped(ExcludedHeaders[I]->getName());
    OS << "\"\n";
  }
  
  for (submodule_const_iterator MI = submodule_begin(), MIEnd = submodule_end();
       MI != MIEnd; ++MI)
    (*MI)->print(OS, Indent + 2);
  
  for (unsigned I = 0, N = Exports.size(); I != N; ++I) {
    OS.indent(Indent + 2);
    OS << "export ";
    if (PCModule *Restriction = Exports[I].getPointer()) {
      OS << Restriction->getFullPCModuleName();
      if (Exports[I].getInt())
        OS << ".*";
    } else {
      OS << "*";
    }
    OS << "\n";
  }

  for (unsigned I = 0, N = UnresolvedExports.size(); I != N; ++I) {
    OS.indent(Indent + 2);
    OS << "export ";
    printPCModuleId(OS, UnresolvedExports[I].Id);
    if (UnresolvedExports[I].Wildcard) {
      if (UnresolvedExports[I].Id.empty())
        OS << "*";
      else
        OS << ".*";
    }
    OS << "\n";
  }

  if (InferSubmodules) {
    OS.indent(Indent + 2);
    if (InferExplicitSubmodules)
      OS << "explicit ";
    OS << "module * {\n";
    if (InferExportWildcard) {
      OS.indent(Indent + 4);
      OS << "export *\n";
    }
    OS.indent(Indent + 2);
    OS << "}\n";
  }
  
  OS.indent(Indent);
  OS << "}\n";
}

void PCModule::dump() const {
  print(llvm::errs());
}


