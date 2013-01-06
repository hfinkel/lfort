// Test this without pch.
// RUN: %lfort_cc1 -include %s -fsyntax-only -verify %s

// Test with pch.
// RUN: %lfort_cc1 -emit-pch -o %t %s
// RUN: %lfort_cc1 -include-pch %t -fsyntax-only -verify %s 

// expected-no-diagnostics

#ifndef HEADER
#define HEADER

// rdar://12627738
namespace rdar12627738 {

class RecyclerTag {
    template <typename T> friend class Recycler;
};

}

#else

namespace rdar12627738 {

template<typename TTag>
class CRN {
    template <typename T> friend class Recycler;
};


template<typename T>
class Recycler {
public:
    Recycler ();
};


template<typename T>
Recycler<T>::Recycler ()
{
}

}

#endif
