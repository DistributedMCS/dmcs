#ifndef CHILD_H
#define CHILD_H

#include "Family.h"
#include "Parent.h"


class Child
{
public:
  Child(const ParentWPtr& p, const std::string n);

  void
  sayHi();

  friend std::ostream&
  operator<< (std::ostream& os, const Child& c);

private:
  ParentWPtr parent;
  std::string name;
};

#endif // CHILD_H

// Local Variables:
// mode: C++
// End:
