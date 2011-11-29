#include "Parent.h"

Parent::Parent(const std::string n)
  : name(n)
{ }


ParentPtr
Parent::create(const std::string& n)
{
  ParentPtr ps(new Parent(n));
  ParentWPtr pw(ps);
  ps->my_weak_ptr = pw;
  return ps;
}


ChildPtr
Parent::createChild(const std::string& cn)
{
  ChildPtr c(new Child(my_weak_ptr, cn));
  children.push_back(c);
  return c;
}


void
Parent::sayHi()
{
  std::cout << name << ": Hi!" << std::endl;
}


std::ostream&
operator<< (std::ostream& os, const Parent& p)
{
  os << p.name << "(";
  for (std::list<ChildPtr>::const_iterator child = p.children.begin();
       child != p.children.end(); ++child)
    {
      os << **child << " ";
    }
  os << ")";
  return os;
}
