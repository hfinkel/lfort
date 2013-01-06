// RUN: %lfort_cc1 %s -E | grep '#pragma x y z'
// RUN: %lfort_cc1 %s -E | grep '#pragma a b c'

_Pragma("x y z")
_Pragma("a b c")

