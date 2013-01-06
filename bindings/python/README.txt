//===----------------------------------------------------------------------===//
// LFort Python Bindings
//===----------------------------------------------------------------------===//

This directory implements Python bindings for LFort.

You may need to alter LD_LIBRARY_PATH so that the LFort library can be
found. The unit tests are designed to be run with 'nosetests'. For example:
--
$ env PYTHONPATH=$(echo ~/llvm/tools/lfort/bindings/python/) \
      LD_LIBRARY_PATH=$(llvm-config --libdir) \
  nosetests -v
tests.cindex.test_index.test_create ... ok
...

OK
--
