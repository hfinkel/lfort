// RUN: %lfort_cc1 -emit-llvm-only %s
struct A
{
A();    
virtual ~A();
};

struct B: A
{
  B();
  ~B();
};

B::B()
{
}

B::~B()
{
}
                         
