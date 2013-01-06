// RUN: %lfort_cc1 -fsyntax-only -verify %s -std=c++11
// expected-no-diagnostics
namespace Test1 {

class A final { };

}
