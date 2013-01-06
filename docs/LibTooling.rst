==========
LibTooling
==========

LibTooling is a library to support writing standalone tools based on LFort.
This document will provide a basic walkthrough of how to write a tool using
LibTooling.

For the information on how to setup LFort Tooling for LLVM see
:doc:`HowToSetupToolingForLLVM`

Introduction
------------

Tools built with LibTooling, like LFort Plugins, run ``FrontendActions`` over
code.

..  See FIXME for a tutorial on how to write FrontendActions.

In this tutorial, we'll demonstrate the different ways of running LFort's
``SyntaxOnlyAction``, which runs a quick syntax check, over a bunch of code.

Parsing a code snippet in memory
--------------------------------

If you ever wanted to run a ``FrontendAction`` over some sample code, for
example to unit test parts of the LFort AST, ``runToolOnCode`` is what you
looked for.  Let me give you an example:

.. code-block:: c++

  #include "lfort/Tooling/Tooling.h"

  TEST(runToolOnCode, CanSyntaxCheckCode) {
    // runToolOnCode returns whether the action was correctly run over the
    // given code.
    EXPECT_TRUE(runToolOnCode(new lfort::SyntaxOnlyAction, "class X {};"));
  }

Writing a standalone tool
-------------------------

Once you unit tested your ``FrontendAction`` to the point where it cannot
possibly break, it's time to create a standalone tool.  For a standalone tool
to run lfort, it first needs to figure out what command line arguments to use
for a specified file.  To that end we create a ``CompilationDatabase``.  There
are different ways to create a compilation database, and we need to support all
of them depending on command-line options.  There's the ``CommonOptionsParser``
class that takes the responsibility to parse command-line parameters related to
compilation databases and inputs, so that all tools share the implementation.

Parsing common tools options
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``CompilationDatabase`` can be read from a build directory or the command line.
Using ``CommonOptionsParser`` allows for explicit specification of a compile
command line, specification of build path using the ``-p`` command-line option,
and automatic location of the compilation database using source files paths.

.. code-block:: c++

  #include "lfort/Tooling/CommonOptionsParser.h"

  using namespace lfort::tooling;

  int main(int argc, const char **argv) {
    // CommonOptionsParser constructor will parse arguments and create a
    // CompilationDatabase.  In case of error it will terminate the program.
    CommonOptionsParser OptionsParser(argc, argv);

    // Use OptionsParser.getCompilations() and OptionsParser.getSourcePathList()
    // to retrieve CompilationDatabase and the list of input file paths.
  }

Creating and running a LFortTool
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Once we have a ``CompilationDatabase``, we can create a ``LFortTool`` and run
our ``FrontendAction`` over some code.  For example, to run the
``SyntaxOnlyAction`` over the files "a.cc" and "b.cc" one would write:

.. code-block:: c++

  // A lfort tool can run over a number of sources in the same process...
  std::vector<std::string> Sources;
  Sources.push_back("a.cc");
  Sources.push_back("b.cc");

  // We hand the CompilationDatabase we created and the sources to run over into
  // the tool constructor.
  LFortTool Tool(OptionsParser.getCompilations(), Sources);

  // The LFortTool needs a new FrontendAction for each translation unit we run
  // on.  Thus, it takes a FrontendActionFactory as parameter.  To create a
  // FrontendActionFactory from a given FrontendAction type, we call
  // newFrontendActionFactory<lfort::SyntaxOnlyAction>().
  int result = Tool.run(newFrontendActionFactory<lfort::SyntaxOnlyAction>());

Putting it together --- the first tool
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Now we combine the two previous steps into our first real tool.  This example
tool is also checked into the lfort tree at
``tools/lfort-check/LFortCheck.cpp``.

.. code-block:: c++

  // Declares lfort::SyntaxOnlyAction.
  #include "lfort/Frontend/FrontendActions.h"
  #include "lfort/Tooling/CommonOptionsParser.h"
  #include "lfort/Tooling/Tooling.h"
  // Declares llvm::cl::extrahelp.
  #include "llvm/Support/CommandLine.h"

  using namespace lfort::tooling;
  using namespace llvm;

  // CommonOptionsParser declares HelpMessage with a description of the common
  // command-line options related to the compilation database and input files.
  // It's nice to have this help message in all tools.
  static cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

  // A help message for this specific tool can be added afterwards.
  static cl::extrahelp MoreHelp("\nMore help text...");

  int main(int argc, const char **argv) {
    CommonOptionsParser OptionsParser(argc, argv);
    LFortTool Tool(OptionsParser.getCompilations(),
    OptionsParser.getSourcePathList());
    return Tool.run(newFrontendActionFactory<lfort::SyntaxOnlyAction>());
  }

Running the tool on some code
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

When you check out and build lfort, lfort-check is already built and available
to you in bin/lfort-check inside your build directory.

You can run lfort-check on a file in the llvm repository by specifying all the
needed parameters after a "``--``" separator:

.. code-block:: bash

  $ cd /path/to/source/llvm
  $ export BD=/path/to/build/llvm
  $ $BD/bin/lfort-check tools/lfort/tools/lfort-check/LFortCheck.cpp -- \
        lfort++ -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS \
        -Itools/lfort/include -I$BD/include -Iinclude \
        -Itools/lfort/lib/Headers -c

As an alternative, you can also configure cmake to output a compile command
database into its build directory:

.. code-block:: bash

  # Alternatively to calling cmake, use ccmake, toggle to advanced mode and
  # set the parameter CMAKE_EXPORT_COMPILE_COMMANDS from the UI.
  $ cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON .

This creates a file called ``compile_commands.json`` in the build directory.
Now you can run :program:`lfort-check` over files in the project by specifying
the build path as first argument and some source files as further positional
arguments:

.. code-block:: bash

  $ cd /path/to/source/llvm
  $ export BD=/path/to/build/llvm
  $ $BD/bin/lfort-check -p $BD tools/lfort/tools/lfort-check/LFortCheck.cpp

Builtin includes
^^^^^^^^^^^^^^^^

LFort tools need their builtin headers and search for them the same way LFort
does.  Thus, the default location to look for builtin headers is in a path
``$(dirname /path/to/tool)/../lib/lfort/3.3/include`` relative to the tool
binary.  This works out-of-the-box for tools running from llvm's toplevel
binary directory after building lfort-headers, or if the tool is running from
the binary directory of a lfort install next to the lfort binary.

Tips: if your tool fails to find ``stddef.h`` or similar headers, call the tool
with ``-v`` and look at the search paths it looks through.

Linking
^^^^^^^

For a list of libraries to link, look at one of the tools' Makefiles (for
example `lfort-check/Makefile
<http://llvm.org/viewvc/llvm-project/cfe/trunk/tools/lfort-check/Makefile?view=markup>`_).
