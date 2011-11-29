#include "Child.h"

Child::Child(const ParentWPtr& p, const std::string n)
  : parent(p), name(n)
{ }


void
Child::sayHi()
{
  std::cout << name << ": Hallo papa!" << std::endl;
  ParentPtr p = parent.lock();
  p->sayHi();
}


std::ostream&
operator<< (std::ostream& os, const Child& c)
{
  os << c.name;
  return os;
}

// Local Variables:
// mode: C++
// End:
