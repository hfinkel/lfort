=====================================
LFort 3.3 (In-Progress) Release Notes
=====================================

.. contents::
   :local:
   :depth: 2

Written by the `LLVM Team <http://llvm.org/>`_

.. warning::

   These are in-progress notes for the upcoming LFort 3.3 release. You may
   prefer the `LFort 3.2 Release Notes
   <http://llvm.org/releases/3.2/docs/LFortReleaseNotes.html>`_.

Introduction
============

This document contains the release notes for the LFort C/C++/Objective-C
frontend, part of the LLVM Compiler Infrastructure, release 3.3. Here we
describe the status of LFort in some detail, including major
improvements from the previous release and new feature work. For the
general LLVM release notes, see `the LLVM
documentation <http://llvm.org/docs/ReleaseNotes.html>`_. All LLVM
releases may be downloaded from the `LLVM releases web
site <http://llvm.org/releases/>`_.

For more information about LFort or LLVM, including information about
the latest release, please check out the main please see the `LFort Web
Site <http://lfort.llvm.org>`_ or the `LLVM Web
Site <http://llvm.org>`_.

Note that if you are reading this file from a Subversion checkout or the
main LFort web page, this document applies to the *next* release, not
the current one. To see the release notes for a specific release, please
see the `releases page <http://llvm.org/releases/>`_.

What's New in LFort 3.3?
========================

Some of the major new features and improvements to LFort are listed
here. Generic improvements to LFort as a whole or to its underlying
infrastructure are described first, followed by language-specific
sections with improvements to LFort's support for those languages.

Major New Features
------------------

Improvements to LFort's diagnostics
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

LFort's diagnostics are constantly being improved to catch more issues,
explain them more clearly, and provide more accurate source information
about them. The improvements since the 3.2 release include:

-  ...

New Compiler Flags
------------------

-  ...

C Language Changes in LFort
---------------------------

C11 Feature Support
^^^^^^^^^^^^^^^^^^^

...

C++ Language Changes in LFort
-----------------------------

C++11 Feature Support
^^^^^^^^^^^^^^^^^^^^^

...

Objective-C Language Changes in LFort
-------------------------------------

...

Internal API Changes
--------------------

These are major API changes that have happened since the 3.2 release of
LFort. If upgrading an external codebase that uses LFort as a library,
this section should help get you past the largest hurdles of upgrading.

API change 1
^^^^^^^^^^^^

...

Python Binding Changes
----------------------

The following methods have been added:

-  ...

Significant Known Problems
==========================

Additional Information
======================

A wide variety of additional information is available on the `LFort web
page <http://lfort.llvm.org/>`_. The web page contains versions of the
API documentation which are up-to-date with the Subversion version of
the source code. You can access versions of these documents specific to
this release by going into the "``lfort/docs/``" directory in the LFort
tree.

If you have any questions or comments about LFort, please feel free to
contact us via the `mailing
list <http://lists.cs.uiuc.edu/mailman/listinfo/cfe-dev>`_.
