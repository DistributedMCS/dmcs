#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testWeakPointer"

#include <boost/test/unit_test.hpp>
#include <boost/shared_ptr.hpp>
#include <iostream>
#include <list>

class BaseState
{
public:
  virtual void
  introduce() = 0;
};



class DerivedState1 : public BaseState
{
public:
  void
  introduce()
  {
    std::cout << "Derived State 1." << std::endl;
  }
};



class DerivedState2 : public BaseState
{
public:
  void
  introduce()
  {
    std::cout << "Derived State 2." << std::endl;
  }
};


typedef boost::shared_ptr<BaseState> BaseStatePtr;
typedef boost::shared_ptr<DerivedState1> DerivedState1Ptr;
typedef boost::shared_ptr<DerivedState2> DerivedState2Ptr;


BOOST_AUTO_TEST_CASE ( testPolymorphismSharedPtr )
{
  BaseStatePtr p1(new DerivedState1);
  p1->introduce();

  BaseStatePtr p2(new DerivedState2);
  p2->introduce();

  DerivedState2Ptr dp2 = boost::dynamic_pointer_cast<DerivedState2>(p2);
  dp2->introduce();
}

// Local Variables:
// mode: C++
// End:
