#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBeliefStateCaching"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


BOOST_AUTO_TEST_CASE ( testBeliefStateCaching )
{
  PartialBeliefStateBufPtr cache(new PartialBeliefStateBuf(3));

  // partial belief states
  std::istringstream iss1("{0 2 4} {0 3 4} {} {}");
  PartialBeliefState* pbs1 = new PartialBeliefState;
  iss1 >> *pbs1;
  BOOST_TEST_MESSAGE("pbs1 = " << *pbs1);
  std::cerr << "pbs1 = " << pbs1 << " " << *pbs1 << std::endl;

  std::istringstream iss2("{0 -2 4} {0 3 4} {} {}");
  PartialBeliefState* pbs2 = new PartialBeliefState;
  iss2 >> *pbs2;
  BOOST_TEST_MESSAGE("pbs2 = " << *pbs2);
  std:: cerr << "pbs2 = " << pbs2 << " " << *pbs2 << std::endl;

  std::istringstream iss3("{0 -1 2 4} {0 2 -3 4} {} {}");
  PartialBeliefState* pbs3 = new PartialBeliefState;
  iss3 >> *pbs3;
  BOOST_TEST_MESSAGE("pbs3 = " << *pbs3);
  std:: cerr << "pbs3 = " << pbs3 << " " << *pbs3 << std::endl;

  std::istringstream iss4("{0 -1 2 4} {0 2 -3 4} {} {}");
  PartialBeliefState* pbs4 = new PartialBeliefState;
  iss4 >> *pbs4;
  BOOST_TEST_MESSAGE("pbs4 = " << *pbs4);
  std:: cerr << "pbs4 = " << pbs4 << " " << *pbs4 << std::endl;

  std::istringstream iss5("{0 -1 2 4} {0 -2 -3 -4} {} {}");
  PartialBeliefState* pbs5 = new PartialBeliefState;
  iss5 >> *pbs5;
  BOOST_TEST_MESSAGE("pbs5 = " << *pbs5);
  std:: cerr << "pbs5 = " << pbs5 << " " << *pbs5 << std::endl;

  std::istringstream iss6("{0 -1 -2 -4} {0 1 2 3 4} {} {}");
  PartialBeliefState* pbs6 = new PartialBeliefState;
  iss6 >> *pbs6;
  BOOST_TEST_MESSAGE("pbs6 = " << *pbs6);
  std:: cerr << "pbs6 = " << pbs6 << " " << *pbs6 << std::endl;

  store(pbs1, cache);
  store(pbs2, cache);
  store(pbs3, cache);

  std::cerr << "CAHCE after 3 stores: " << std::endl << *cache << std::endl;

  store(pbs4, cache);

  std::cerr << "CAHCE after 4 stores: " << std::endl << *cache << std::endl;

  bool null_pbs4 = pbs4 == 0;

  BOOST_CHECK_EQUAL(null_pbs4, true);

  store(pbs5, cache);

  std::cerr << "CAHCE after 5 stores: " << std::endl << *cache << std::endl;

  std::cerr << "Test stability of cache" << std::endl;
  pbs2 = pbs6;
  
  std:: cerr << "pbs2 should change = " << pbs2 << " " << *pbs2 << std::endl;
  std::cerr << "CAHCE should not change: " << std::endl << *cache << std::endl;

  // Uncomment the following lines to see the effect of deleting temp_pbs in store(). 
  // We should get an error here. Otw we are doomed because of memory leaks
  //std::cerr << "Try to access pbs1: " << std::endl;
  //std::cerr << "pbs1 = " << *pbs1 << std::endl;
}
