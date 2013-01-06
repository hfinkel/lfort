//===-- CGBuilder.h - Choose IRBuilder implementation  ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_CODEGEN_CGBUILDER_H
#define LFORT_CODEGEN_CGBUILDER_H

#include "llvm/IR/IRBuilder.h"

namespace lfort {
namespace CodeGen {

// Don't preserve names on values in an optimized build.
#ifdef NDEBUG
typedef llvm::IRBuilder<false> CGBuilderTy;
#else
typedef llvm::IRBuilder<> CGBuilderTy;
#endif

}  // end namespace CodeGen
}  // end namespace lfort

#endif
