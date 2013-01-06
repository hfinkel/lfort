/* RUN: %lfort_cc1 -E %s -x c++ | grep block_1
   RUN: %lfort_cc1 -E %s -x c++ | not grep block_2
   RUN: %lfort_cc1 -E %s -x c | not grep block
   RUN: %lfort_cc1 -E %s -x c++ -verify -Wundef
*/
// expected-no-diagnostics

#if true
block_1
#endif

#if false
block_2
#endif

