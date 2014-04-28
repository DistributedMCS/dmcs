#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testWeakPointer"

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <iostream>
#include <list>

#include "Child.h"
#include "Parent.h"

BOOST_AUTO_TEST_CASE ( testWeakPointer )
{
  ParentPtr p = Parent::create("Homer");
  ChildPtr bart = p->createChild("Bart");
  ChildPtr lisa = p->createChild("Lisa");
  ChildPtr maggie = p->createChild("Maggie");

  std::cout << "The Simpson family: " << *p << std::endl;

  lisa->sayHi();
}

// Local Variables:
// mode: C++
// End:
