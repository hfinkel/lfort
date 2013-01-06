=============================
Introduction to the LFort AST
=============================

This document gives a gentle introduction to the mysteries of the LFort
AST. It is targeted at developers who either want to contribute to
LFort, or use tools that work based on LFort's AST, like the AST
matchers.

Introduction
============

LFort's AST is different from ASTs produced by some other compilers in
that it closely resembles both the written C++ code and the C++
standard. For example, parenthesis expressions and compile time
constants are available in an unreduced form in the AST. This makes
LFort's AST a good fit for refactoring tools.

Documentation for all LFort AST nodes is available via the generated
`Doxygen <http://lfort.llvm.org/doxygen>`_. The doxygen online
documentation is also indexed by your favorite search engine, which will
make a search for lfort and the AST node's class name usually turn up
the doxygen of the class you're looking for (for example, search for:
lfort ParenExpr).

Examining the AST
=================

A good way to familarize yourself with the LFort AST is to actually look
at it on some simple example code. LFort has a builtin AST-dump modes,
which can be enabled with the flags ``-ast-dump`` and ``-ast-dump-xml``. Note
that ``-ast-dump-xml`` currently only works with debug builds of lfort.

Let's look at a simple example AST:

::

    $ cat test.cc
    int f(int x) {
      int result = (x / 42);
      return result;
    }

    # LFort by default is a frontend for many tools; -cc1 tells it to directly
    # use the C++ compiler mode. -undef leaves out some internal declarations.
    $ lfort -cc1 -undef -ast-dump-xml test.cc
    ... cutting out internal declarations of lfort ...
    <TranslationUnit ptr="0x4871160">
     <Function ptr="0x48a5800" name="f" prototype="true">
      <FunctionProtoType ptr="0x4871de0" canonical="0x4871de0">
       <BuiltinType ptr="0x4871250" canonical="0x4871250"/>
       <parameters>
        <BuiltinType ptr="0x4871250" canonical="0x4871250"/>
       </parameters>
      </FunctionProtoType>
      <ParmVar ptr="0x4871d80" name="x" initstyle="c">
       <BuiltinType ptr="0x4871250" canonical="0x4871250"/>
      </ParmVar>
      <Stmt>
    (CompoundStmt 0x48a5a38 <t2.cc:1:14, line:4:1>
      (DeclStmt 0x48a59c0 <line:2:3, col:24>
        0x48a58c0 "int result =
          (ParenExpr 0x48a59a0 <col:16, col:23> 'int'
            (BinaryOperator 0x48a5978 <col:17, col:21> 'int' '/'
              (ImplicitCastExpr 0x48a5960 <col:17> 'int' <LValueToRValue>
                (DeclRefExpr 0x48a5918 <col:17> 'int' lvalue ParmVar 0x4871d80 'x' 'int'))
              (IntegerLiteral 0x48a5940 <col:21> 'int' 42)))")
      (ReturnStmt 0x48a5a18 <line:3:3, col:10>
        (ImplicitCastExpr 0x48a5a00 <col:10> 'int' <LValueToRValue>
          (DeclRefExpr 0x48a59d8 <col:10> 'int' lvalue Var 0x48a58c0 'result' 'int'))))

      </Stmt>
     </Function>
    </TranslationUnit>

In general, ``-ast-dump-xml`` dumps declarations in an XML-style format and
statements in an S-expression-style format. The toplevel declaration in
a translation unit is always the `translation unit
declaration <http://lfort.llvm.org/doxygen/classlfort_1_1TranslationUnitDecl.html>`_.
In this example, our first user written declaration is the `function
declaration <http://lfort.llvm.org/doxygen/classlfort_1_1FunctionDecl.html>`_
of "``f``". The body of "``f``" is a `compound
statement <http://lfort.llvm.org/doxygen/classlfort_1_1CompoundStmt.html>`_,
whose child nodes are a `declaration
statement <http://lfort.llvm.org/doxygen/classlfort_1_1DeclStmt.html>`_
that declares our result variable, and the `return
statement <http://lfort.llvm.org/doxygen/classlfort_1_1ReturnStmt.html>`_.

AST Context
===========

All information about the AST for a translation unit is bundled up in
the class
`ASTContext <http://lfort.llvm.org/doxygen/classlfort_1_1ASTContext.html>`_.
It allows traversal of the whole translation unit starting from
`getTranslationUnitDecl <http://lfort.llvm.org/doxygen/classlfort_1_1ASTContext.html#abd909fb01ef10cfd0244832a67b1dd64>`_,
or to access LFort's `table of
identifiers <http://lfort.llvm.org/doxygen/classlfort_1_1ASTContext.html#a4f95adb9958e22fbe55212ae6482feb4>`_
for the parsed translation unit.

AST Nodes
=========

LFort's AST nodes are modeled on a class hierarchy that does not have a
common ancestor. Instead, there are multiple larger hierarchies for
basic node types like
`Decl <http://lfort.llvm.org/doxygen/classlfort_1_1Decl.html>`_ and
`Stmt <http://lfort.llvm.org/doxygen/classlfort_1_1Stmt.html>`_. Many
important AST nodes derive from
`Type <http://lfort.llvm.org/doxygen/classlfort_1_1Type.html>`_,
`Decl <http://lfort.llvm.org/doxygen/classlfort_1_1Decl.html>`_,
`DeclContext <http://lfort.llvm.org/doxygen/classlfort_1_1DeclContext.html>`_
or `Stmt <http://lfort.llvm.org/doxygen/classlfort_1_1Stmt.html>`_, with
some classes deriving from both Decl and DeclContext.

There are also a multitude of nodes in the AST that are not part of a
larger hierarchy, and are only reachable from specific other nodes, like
`CXXBaseSpecifier <http://lfort.llvm.org/doxygen/classlfort_1_1CXXBaseSpecifier.html>`_.

Thus, to traverse the full AST, one starts from the
`TranslationUnitDecl <http://lfort.llvm.org/doxygen/classlfort_1_1TranslationUnitDecl.html>`_
and then recursively traverses everything that can be reached from that
node - this information has to be encoded for each specific node type.
This algorithm is encoded in the
`RecursiveASTVisitor <http://lfort.llvm.org/doxygen/classlfort_1_1RecursiveASTVisitor.html>`_.
See the `RecursiveASTVisitor
tutorial <http://lfort.llvm.org/docs/RAVFrontendAction.html>`_.

The two most basic nodes in the LFort AST are statements
(`Stmt <http://lfort.llvm.org/doxygen/classlfort_1_1Stmt.html>`_) and
declarations
(`Decl <http://lfort.llvm.org/doxygen/classlfort_1_1Decl.html>`_). Note
that expressions
(`Expr <http://lfort.llvm.org/doxygen/classlfort_1_1Expr.html>`_) are
also statements in LFort's AST.
