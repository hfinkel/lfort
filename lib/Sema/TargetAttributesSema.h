//===--- TargetAttributesSema.h - Semantic Analysis For Target Attributes -===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#ifndef LFORT_SEMA_TARGETSEMA_H
#define LFORT_SEMA_TARGETSEMA_H

namespace lfort {
  class Scope;
  class Decl;
  class AttributeList;
  class Sema;

  class TargetAttributesSema {
  public:
    virtual ~TargetAttributesSema();
    virtual bool ProcessDeclAttribute(Scope *scope, Decl *D,
                                      const AttributeList &Attr, Sema &S) const;
  };
}

#endif
