#include "mcs/BeliefState.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBeliefState"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


BOOST_AUTO_TEST_CASE ( testBeliefState )
{
  // partial belief states
  std::istringstream iss1("21 25 0 0");
  PartialBeliefState pbs1;
  iss1 >> pbs1;
  BOOST_TEST_MESSAGE("pbs1 = " << pbs1);

  std::istringstream iss2("{0 2 4} {0 3 4} {} {}");
  PartialBeliefState pbs2;
  iss2 >> pbs2;
  BOOST_TEST_MESSAGE("pbs2 = " << pbs2);

  bool pb_equal = (pbs1 == pbs2);
  BOOST_CHECK_EQUAL(pb_equal, true);

  std::istringstream iss3("{-1 2 4} {2 -3 4} {} {}");
  PartialBeliefState pbs3;
  iss3 >> pbs3;
  BOOST_TEST_MESSAGE("pbs3 = " << pbs3);

  // belief states
  std::istringstream iss4("1 9 25 0");
  BeliefStatePtr bs4;
  iss4 >> bs4;
  BOOST_TEST_MESSAGE("bs4  = " << bs4);

  std::istringstream iss5("{0} {0 3} {0 3 4} {}");
  BeliefStatePtr bs5;
  iss5 >> bs5;
  BOOST_TEST_MESSAGE("bs5  = " << bs5);
  bool b_equal = (*bs4 == *bs5);
  BOOST_CHECK_EQUAL(b_equal, true);
}
