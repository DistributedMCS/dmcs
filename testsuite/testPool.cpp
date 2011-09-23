#include "dmcs/Log.h"
#include "mcs/BaseType.h"
#include "mcs/Pool.h"



#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testPool"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;



struct MyType : public BaseType
{
  MyType(const std::string& n, std::size_t a)
    : BaseType(), name(n), age(a)
  { }

  std::string name;
  std::size_t age;
};



BOOST_AUTO_TEST_CASE ( testPool )
{
  Pool pool(5, sizeof(MyType));
  
  MyType* mt;
  std::string name = "tweety";
  std::size_t age = 5;

  mt = new (&pool) MyType(name, age);

  BOOST_CHECK_EQUAL(mt->name, name);
  BOOST_CHECK_EQUAL(mt->age, age);
}
