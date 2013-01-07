//===--- LangStandard.h -----------------------------------------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_FRONTEND_LANGSTANDARD_H
#define LLVM_LFORT_FRONTEND_LANGSTANDARD_H

#include "lfort/Basic/LLVM.h"
#include "llvm/ADT/StringRef.h"

namespace lfort {

namespace frontend {

enum LangFeatures {
  LineComment = (1 << 0),
  C89 = (1 << 1),
  C99 = (1 << 2),
  C11 = (1 << 3),
  CPlusPlus = (1 << 4),
  CPlusPlus11 = (1 << 5),
  CPlusPlus1y = (1 << 6),
  Digraphs = (1 << 7),
  GNUMode = (1 << 8),
  HexFloat = (1 << 9),
  ImplicitInt = (1 << 10),
  F77 = (1 << 11), // includes MIL-STD-1753
  F90 = (1 << 12),
  F95 = (1 << 13),
  F03 = (1 << 14),
  F08 = (1 << 15)
};

}

/// LangStandard - Information about the properties of a particular language
/// standard.
struct LangStandard {
  enum Kind {
#define LANGSTANDARD(id, name, desc, features) \
    lang_##id,
#include "lfort/Frontend/LangStandards.def"
    lang_unspecified
  };

  const char *ShortName;
  const char *Description;
  unsigned Flags;

public:
  /// getName - Get the name of this standard.
  const char *getName() const { return ShortName; }

  /// getDescription - Get the description of this standard.
  const char *getDescription() const { return Description; }

  /// Language supports '//' comments.
  bool hasLineComments() const { return Flags & frontend::LineComment; }

  /// isC89 - Language is a superset of C89.
  bool isC89() const { return Flags & frontend::C89; }

  /// isC99 - Language is a superset of C99.
  bool isC99() const { return Flags & frontend::C99; }

  /// isC11 - Language is a superset of C11.
  bool isC11() const { return Flags & frontend::C11; }

  /// isCPlusPlus - Language is a C++ variant.
  bool isCPlusPlus() const { return Flags & frontend::CPlusPlus; }

  /// isCPlusPlus11 - Language is a C++0x variant.
  bool isCPlusPlus11() const { return Flags & frontend::CPlusPlus11; }

  /// isCPlusPlus1y - Language is a C++1y variant.
  bool isCPlusPlus1y() const { return Flags & frontend::CPlusPlus1y; }

  /// hasDigraphs - Language supports digraphs.
  bool hasDigraphs() const { return Flags & frontend::Digraphs; }

  /// isGNUMode - Language includes GNU extensions.
  bool isGNUMode() const { return Flags & frontend::GNUMode; }

  /// hasHexFloats - Language supports hexadecimal float constants.
  bool hasHexFloats() const { return Flags & frontend::HexFloat; }

  /// hasImplicitInt - Language allows variables to be typed as int implicitly.
  bool hasImplicitInt() const { return Flags & frontend::ImplicitInt; }

  /// isF77 - Language is a superset of Fortran 1977.
  bool isF77() const { return Flags & frontend::F77; }

  /// isF90 - Language is a superset of Fortran 1990.
  bool isF90() const { return Flags & frontend::F90; }

  /// isF95 - Language is a superset of Fortran 1995.
  bool isF95() const { return Flags & frontend::F95; }

  /// isF03 - Language is a superset of Fortran 2003.
  bool isF03() const { return Flags & frontend::F03; }

  /// isF08 - Language is a superset of Fortran 2008.
  bool isF08() const { return Flags & frontend::F08; }

  static const LangStandard &getLangStandardForKind(Kind K);
  static const LangStandard *getLangStandardForName(StringRef Name);
};

}  // end namespace lfort

#endif
