//===--- GeneratePCH.cpp - Sema Consumer for PCH Generation -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//  This file defines the PCHGenerator, which as a SemaConsumer that generates
//  a PCH file.
//
//===----------------------------------------------------------------------===//

#include "lfort/Serialization/ASTWriter.h"
#include "lfort/AST/ASTConsumer.h"
#include "lfort/AST/ASTContext.h"
#include "lfort/Basic/FileManager.h"
#include "lfort/Lex/Preprocessor.h"
#include "lfort/Sema/SemaConsumer.h"
#include "llvm/Bitcode/BitstreamWriter.h"
#include "llvm/Support/raw_ostream.h"
#include <string>

using namespace lfort;

PCHGenerator::PCHGenerator(const Preprocessor &PP,
                           StringRef OutputFile,
                           lfort::Module *Module,
                           StringRef isysroot,
                           raw_ostream *OS)
  : PP(PP), OutputFile(OutputFile), Module(Module), 
    isysroot(isysroot.str()), Out(OS), 
    SemaPtr(0), Stream(Buffer), Writer(Stream) {
}

PCHGenerator::~PCHGenerator() {
}

void PCHGenerator::HandleTranslationUnit(ASTContext &Ctx) {
  if (PP.getDiagnostics().hasErrorOccurred())
    return;
  
  // Emit the PCH file
  assert(SemaPtr && "No Sema?");
  Writer.WriteAST(*SemaPtr, OutputFile, Module, isysroot);

  // Write the generated bitstream to "Out".
  Out->write((char *)&Buffer.front(), Buffer.size());

  // Make sure it hits disk now.
  Out->flush();

  // Free up some memory, in case the process is kept alive.
  Buffer.clear();
}

PPMutationListener *PCHGenerator::GetPPMutationListener() {
  return &Writer;
}

ASTMutationListener *PCHGenerator::GetASTMutationListener() {
  return &Writer;
}

ASTDeserializationListener *PCHGenerator::GetASTDeserializationListener() {
  return &Writer;
}
