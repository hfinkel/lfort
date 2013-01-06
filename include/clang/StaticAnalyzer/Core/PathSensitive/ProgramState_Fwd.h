//== ProgramState_Fwd.h - Incomplete declarations of ProgramState -*- C++ -*--=/
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_PROGRAMSTATE_FWD_H
#define LLVM_LFORT_PROGRAMSTATE_FWD_H

#include "lfort/Basic/LLVM.h"
#include "llvm/ADT/IntrusiveRefCntPtr.h"

namespace lfort {
namespace ento {
  class ProgramState;
  class ProgramStateManager;
  void ProgramStateRetain(const ProgramState *state);
  void ProgramStateRelease(const ProgramState *state);
}
}

namespace llvm {
  template <> struct IntrusiveRefCntPtrInfo<const lfort::ento::ProgramState> {
    static void retain(const lfort::ento::ProgramState *state) {
      lfort::ento::ProgramStateRetain(state);
    }
    static void release(const lfort::ento::ProgramState *state) {
      lfort::ento::ProgramStateRelease(state);
    }
  };
}

namespace lfort {
namespace ento {
  typedef IntrusiveRefCntPtr<const ProgramState> ProgramStateRef;
}
}

#endif

