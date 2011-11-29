#ifndef FAMILY_H
#define FAMILY_H

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class Child;
class Parent;

typedef boost::shared_ptr<Child> ChildPtr;
typedef boost::shared_ptr<Parent> ParentPtr;
typedef boost::weak_ptr<Parent> ParentWPtr;

#endif // FAMILY_H
