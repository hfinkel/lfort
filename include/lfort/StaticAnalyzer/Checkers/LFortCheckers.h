//===--- LFortCheckers.h - Provides builtin checkers ------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_STATICANALYZER_CHECKERS_LFORTCHECKERS_H
#define LLVM_LFORT_STATICANALYZER_CHECKERS_LFORTCHECKERS_H

namespace lfort {
namespace ento {
class CheckerRegistry;

void registerBuiltinCheckers(CheckerRegistry &registry);

} // end namespace ento
} // end namespace lfort

#endif
