//===--- TargetBuiltins.h - Target specific builtin IDs -------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Enumerates target-specific builtins in their own namespaces within
/// namespace ::lfort.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_BASIC_TARGET_BUILTINS_H
#define LLVM_LFORT_BASIC_TARGET_BUILTINS_H

#include "lfort/Basic/Builtins.h"
#undef PPC

namespace lfort {

  /// \brief ARM builtins
  namespace ARM {
    enum {
        LastTIBuiltin = lfort::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "lfort/Basic/BuiltinsARM.def"
        LastTSBuiltin
    };
  }

  /// \brief PPC builtins
  namespace PPC {
    enum {
        LastTIBuiltin = lfort::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "lfort/Basic/BuiltinsPPC.def"
        LastTSBuiltin
    };
  }

  /// \brief NVPTX builtins
  namespace NVPTX {
    enum {
        LastTIBuiltin = lfort::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "lfort/Basic/BuiltinsNVPTX.def"
        LastTSBuiltin
    };
  }


  /// \brief X86 builtins
  namespace X86 {
    enum {
        LastTIBuiltin = lfort::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "lfort/Basic/BuiltinsX86.def"
        LastTSBuiltin
    };
  }

  /// \brief Flags to identify the types for overloaded Neon builtins.
  ///
  /// These must be kept in sync with the flags in utils/TableGen/NeonEmitter.h.
  class NeonTypeFlags {
    enum {
      EltTypeMask = 0xf,
      UnsignedFlag = 0x10,
      QuadFlag = 0x20
    };
    uint32_t Flags;

  public:
    enum EltType {
      Int8,
      Int16,
      Int32,
      Int64,
      Poly8,
      Poly16,
      Float16,
      Float32
    };

    NeonTypeFlags(unsigned F) : Flags(F) {}
    NeonTypeFlags(EltType ET, bool IsUnsigned, bool IsQuad) : Flags(ET) {
      if (IsUnsigned)
        Flags |= UnsignedFlag;
      if (IsQuad)
        Flags |= QuadFlag;
    }

    EltType getEltType() const { return (EltType)(Flags & EltTypeMask); }
    bool isPoly() const {
      EltType ET = getEltType();
      return ET == Poly8 || ET == Poly16;
    }
    bool isUnsigned() const { return (Flags & UnsignedFlag) != 0; }
    bool isQuad() const { return (Flags & QuadFlag) != 0; }
  };

  /// \brief Hexagon builtins
  namespace Hexagon {
    enum {
        LastTIBuiltin = lfort::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "lfort/Basic/BuiltinsHexagon.def"
        LastTSBuiltin
    };
  }

  /// \brief MIPS builtins
  namespace Mips {
    enum {
        LastTIBuiltin = lfort::Builtin::FirstTSBuiltin-1,
#define BUILTIN(ID, TYPE, ATTRS) BI##ID,
#include "lfort/Basic/BuiltinsMips.def"
        LastTSBuiltin
    };
  }
} // end namespace lfort.

#endif
