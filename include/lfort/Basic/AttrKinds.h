//===----- Attr.h - Enum values for C Attribute Kinds ----------*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
///
/// \file
/// \brief Defines the lfort::attr::Kind enum.
///
//===----------------------------------------------------------------------===//

#ifndef LLVM_LFORT_ATTRKINDS_H
#define LLVM_LFORT_ATTRKINDS_H

namespace lfort {

namespace attr {

// \brief A list of all the recognized kinds of attributes.
enum Kind {
#define ATTR(X) X,
#define LAST_INHERITABLE_ATTR(X) X, LAST_INHERITABLE = X,
#define LAST_INHERITABLE_PARAM_ATTR(X) X, LAST_INHERITABLE_PARAM = X,
#include "lfort/Basic/AttrList.inc"
  NUM_ATTRS
};

} // end namespace attr
} // end namespace lfort

#endif
