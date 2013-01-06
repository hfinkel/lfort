//===--- CompilationDatabasePluginRegistry.h - ------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_TOOLING_COMPILATION_DATABASE_PLUGIN_REGISTRY_H
#define LLVM_LFORT_TOOLING_COMPILATION_DATABASE_PLUGIN_REGISTRY_H

#include "lfort/Tooling/CompilationDatabase.h"
#include "llvm/Support/Registry.h"

namespace lfort {
namespace tooling {

class CompilationDatabasePlugin;

typedef llvm::Registry<CompilationDatabasePlugin>
    CompilationDatabasePluginRegistry;

} // end namespace tooling
} // end namespace lfort

#endif // LLVM_LFORT_TOOLING_COMPILATION_DATABASE_PLUGIN_REGISTRY_H
