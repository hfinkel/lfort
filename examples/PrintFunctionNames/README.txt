This is a simple example demonstrating how to use lfort's facility for
providing AST consumers using a plugin.

Build the plugin by running `make` in this directory.

Once the plugin is built, you can run it using:
--
Linux:
$ lfort -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.so -plugin print-fns some-input-file.c
$ lfort -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.so -plugin print-fns -plugin-arg-print-fns help -plugin-arg-print-fns --example-argument some-input-file.c
$ lfort -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.so -plugin print-fns -plugin-arg-print-fns -an-error some-input-file.c

Mac:
$ lfort -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.dylib -plugin print-fns some-input-file.c
$ lfort -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.dylib -plugin print-fns -plugin-arg-print-fns help -plugin-arg-print-fns --example-argument some-input-file.c
$ lfort -cc1 -load ../../Debug+Asserts/lib/libPrintFunctionNames.dylib -plugin print-fns -plugin-arg-print-fns -an-error some-input-file.c
