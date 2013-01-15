//===--- PCModuleManager.cpp - PCModule Manager ---------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the PCModuleManager class, which manages a set of loaded
//  modules for the ASTReader.
//
//===----------------------------------------------------------------------===//
#include "lfort/Serialization/PCModuleManager.h"
#include "llvm/Support/MemoryBuffer.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/system_error.h"

#ifndef NDEBUG
#include "llvm/Support/GraphWriter.h"
#endif

using namespace lfort;
using namespace serialization;

PCModuleFile *PCModuleManager::lookup(StringRef Name) {
  const FileEntry *Entry = FileMgr.getFile(Name);
  return PCModules[Entry];
}

llvm::MemoryBuffer *PCModuleManager::lookupBuffer(StringRef Name) {
  const FileEntry *Entry = FileMgr.getFile(Name);
  return InMemoryBuffers[Entry];
}

std::pair<PCModuleFile *, bool>
PCModuleManager::addPCModule(StringRef FileName, PCModuleKind Type,
                         SourceLocation ImportLoc, PCModuleFile *ImportedBy,
                         unsigned Generation, std::string &ErrorStr) {
  const FileEntry *Entry = FileMgr.getFile(FileName);
  if (!Entry && FileName != "-") {
    ErrorStr = "file not found";
    return std::make_pair(static_cast<PCModuleFile*>(0), false);
  }
  
  // Check whether we already loaded this module, before 
  PCModuleFile *&PCModuleEntry = PCModules[Entry];
  bool NewPCModule = false;
  if (!PCModuleEntry) {
    // Allocate a new module.
    PCModuleFile *New = new PCModuleFile(Type, Generation);
    New->FileName = FileName.str();
    New->File = Entry;
    New->ImportLoc = ImportLoc;
    Chain.push_back(New);
    NewPCModule = true;
    PCModuleEntry = New;

    // Load the contents of the module
    if (llvm::MemoryBuffer *Buffer = lookupBuffer(FileName)) {
      // The buffer was already provided for us.
      assert(Buffer && "Passed null buffer");
      New->Buffer.reset(Buffer);
    } else {
      // Open the AST file.
      llvm::error_code ec;
      if (FileName == "-") {
        ec = llvm::MemoryBuffer::getSTDIN(New->Buffer);
        if (ec)
          ErrorStr = ec.message();
      } else
        New->Buffer.reset(FileMgr.getBufferForFile(FileName, &ErrorStr));
      
      if (!New->Buffer)
        return std::make_pair(static_cast<PCModuleFile*>(0), false);
    }
    
    // Initialize the stream
    New->StreamFile.init((const unsigned char *)New->Buffer->getBufferStart(),
                         (const unsigned char *)New->Buffer->getBufferEnd());     }
  
  if (ImportedBy) {
    PCModuleEntry->ImportedBy.insert(ImportedBy);
    ImportedBy->Imports.insert(PCModuleEntry);
  } else {
    if (!PCModuleEntry->DirectlyImported)
      PCModuleEntry->ImportLoc = ImportLoc;
    
    PCModuleEntry->DirectlyImported = true;
  }
  
  return std::make_pair(PCModuleEntry, NewPCModule);
}

namespace {
  /// \brief Predicate that checks whether a module file occurs within
  /// the given set.
  class IsInPCModuleFileSet : public std::unary_function<PCModuleFile *, bool> {
    llvm::SmallPtrSet<PCModuleFile *, 4> &Removed;

  public:
    IsInPCModuleFileSet(llvm::SmallPtrSet<PCModuleFile *, 4> &Removed)
    : Removed(Removed) { }

    bool operator()(PCModuleFile *MF) const {
      return Removed.count(MF);
    }
  };
}

void PCModuleManager::removePCModules(PCModuleIterator first, PCModuleIterator last) {
  if (first == last)
    return;

  // Collect the set of module file pointers that we'll be removing.
  llvm::SmallPtrSet<PCModuleFile *, 4> victimSet(first, last);

  // Remove any references to the now-destroyed modules.
  IsInPCModuleFileSet checkInSet(victimSet);
  for (unsigned i = 0, n = Chain.size(); i != n; ++i) {
    Chain[i]->ImportedBy.remove_if(checkInSet);
  }

  // Delete the modules and erase them from the various structures.
  for (PCModuleIterator victim = first; victim != last; ++victim) {
    PCModules.erase((*victim)->File);
    delete *victim;
  }

  // Remove the modules from the chain.
  Chain.erase(first, last);
}

void PCModuleManager::addInMemoryBuffer(StringRef FileName, 
                                      llvm::MemoryBuffer *Buffer) {
  
  const FileEntry *Entry = FileMgr.getVirtualFile(FileName, 
                                                  Buffer->getBufferSize(), 0);
  InMemoryBuffers[Entry] = Buffer;
}

PCModuleManager::PCModuleManager(FileManager &FileMgr) : FileMgr(FileMgr) { }

PCModuleManager::~PCModuleManager() {
  for (unsigned i = 0, e = Chain.size(); i != e; ++i)
    delete Chain[e - i - 1];
}

void PCModuleManager::visit(bool (*Visitor)(PCModuleFile &M, void *UserData), 
                          void *UserData) {
  unsigned N = size();
  
  // Record the number of incoming edges for each module. When we
  // encounter a module with no incoming edges, push it into the queue
  // to seed the queue.
  SmallVector<PCModuleFile *, 4> Queue;
  Queue.reserve(N);
  llvm::DenseMap<PCModuleFile *, unsigned> UnusedIncomingEdges; 
  for (PCModuleIterator M = begin(), MEnd = end(); M != MEnd; ++M) {
    if (unsigned Size = (*M)->ImportedBy.size())
      UnusedIncomingEdges[*M] = Size;
    else
      Queue.push_back(*M);
  }
  
  llvm::SmallPtrSet<PCModuleFile *, 4> Skipped;
  unsigned QueueStart = 0;
  while (QueueStart < Queue.size()) {
    PCModuleFile *CurrentPCModule = Queue[QueueStart++];
    
    // Check whether this module should be skipped.
    if (Skipped.count(CurrentPCModule))
      continue;
    
    if (Visitor(*CurrentPCModule, UserData)) {
      // The visitor has requested that cut off visitation of any
      // module that the current module depends on. To indicate this
      // behavior, we mark all of the reachable modules as having N
      // incoming edges (which is impossible otherwise).
      SmallVector<PCModuleFile *, 4> Stack;
      Stack.push_back(CurrentPCModule);
      Skipped.insert(CurrentPCModule);
      while (!Stack.empty()) {
        PCModuleFile *NextPCModule = Stack.back();
        Stack.pop_back();
        
        // For any module that this module depends on, push it on the
        // stack (if it hasn't already been marked as visited).
        for (llvm::SetVector<PCModuleFile *>::iterator 
             M = NextPCModule->Imports.begin(),
             MEnd = NextPCModule->Imports.end();
             M != MEnd; ++M) {
          if (Skipped.insert(*M))
            Stack.push_back(*M);
        }
      }
      continue;
    }
    
    // For any module that this module depends on, push it on the
    // stack (if it hasn't already been marked as visited).
    for (llvm::SetVector<PCModuleFile *>::iterator M = CurrentPCModule->Imports.begin(),
         MEnd = CurrentPCModule->Imports.end();
         M != MEnd; ++M) {
      
      // Remove our current module as an impediment to visiting the
      // module we depend on. If we were the last unvisited module
      // that depends on this particular module, push it into the
      // queue to be visited.
      unsigned &NumUnusedEdges = UnusedIncomingEdges[*M];
      if (NumUnusedEdges && (--NumUnusedEdges == 0))
        Queue.push_back(*M);
    }
  }
}

/// \brief Perform a depth-first visit of the current module.
static bool visitDepthFirst(PCModuleFile &M, 
                            bool (*Visitor)(PCModuleFile &M, bool Preorder, 
                                            void *UserData), 
                            void *UserData,
                            llvm::SmallPtrSet<PCModuleFile *, 4> &Visited) {
  // Preorder visitation
  if (Visitor(M, /*Preorder=*/true, UserData))
    return true;
  
  // Visit children
  for (llvm::SetVector<PCModuleFile *>::iterator IM = M.Imports.begin(),
       IMEnd = M.Imports.end();
       IM != IMEnd; ++IM) {
    if (!Visited.insert(*IM))
      continue;
    
    if (visitDepthFirst(**IM, Visitor, UserData, Visited))
      return true;
  }  
  
  // Postorder visitation
  return Visitor(M, /*Preorder=*/false, UserData);
}

void PCModuleManager::visitDepthFirst(bool (*Visitor)(PCModuleFile &M, bool Preorder, 
                                                    void *UserData), 
                                    void *UserData) {
  llvm::SmallPtrSet<PCModuleFile *, 4> Visited;
  for (unsigned I = 0, N = Chain.size(); I != N; ++I) {
    if (!Visited.insert(Chain[I]))
      continue;
    
    if (::visitDepthFirst(*Chain[I], Visitor, UserData, Visited))
      return;
  }
}

#ifndef NDEBUG
namespace llvm {
  template<>
  struct GraphTraits<PCModuleManager> {
    typedef PCModuleFile NodeType;
    typedef llvm::SetVector<PCModuleFile *>::const_iterator ChildIteratorType;
    typedef PCModuleManager::PCModuleConstIterator nodes_iterator;
    
    static ChildIteratorType child_begin(NodeType *Node) {
      return Node->Imports.begin();
    }

    static ChildIteratorType child_end(NodeType *Node) {
      return Node->Imports.end();
    }
    
    static nodes_iterator nodes_begin(const PCModuleManager &Manager) {
      return Manager.begin();
    }
    
    static nodes_iterator nodes_end(const PCModuleManager &Manager) {
      return Manager.end();
    }
  };
  
  template<>
  struct DOTGraphTraits<PCModuleManager> : public DefaultDOTGraphTraits {
    explicit DOTGraphTraits(bool IsSimple = false)
      : DefaultDOTGraphTraits(IsSimple) { }
    
    static bool renderGraphFromBottomUp() {
      return true;
    }

    std::string getNodeLabel(PCModuleFile *M, const PCModuleManager&) {
      return llvm::sys::path::stem(M->FileName);
    }
  };
}

void PCModuleManager::viewGraph() {
  llvm::ViewGraph(*this, "PCModules");
}
#endif
