#ifndef PARENT_H
#define PARENT_H

#include <list>
#include "Family.h"
#include "Child.h"

class Child;

class Parent
{
public:
  static ParentPtr create(const std::string& n);

  ChildPtr
  createChild(const std::string& cn);

  void
  sayHi();

  friend std::ostream&
  operator<< (std::ostream& os, const Parent& p);

private:
  // make the ctor private so that we can create shared_ and weak_ptr
  // at construction time.
  Parent(const std::string n);

private:
  ParentWPtr my_weak_ptr;       // store a weak_ptr to myself for creating my children
  std::list<ChildPtr> children;
  std::string name;
};



#endif // PARENT_H
