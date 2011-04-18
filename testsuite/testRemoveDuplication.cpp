#include "mcs/BeliefState.h"
#include "dmcs/ModelSessionId.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testRemoveDuplication"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


BOOST_AUTO_TEST_CASE ( testRemoveDuplication )
{
  // partial belief states
  std::istringstream iss1("21 25 0 0");
  PartialBeliefState pbs1;
  iss1 >> pbs1;

  // partial belief states
  std::istringstream iss2("21 25 0 0");
  PartialBeliefState pbs2;
  iss2 >> pbs2;

  std::istringstream iss3("{0 2 4} {0 3 4} {} {}");
  PartialBeliefState pbs3;
  iss3 >> pbs3;

  std::istringstream iss4("{-1 2 4} {2 -3 4} {} {}");
  PartialBeliefState pbs4;
  iss4 >> pbs4;

  std::istringstream iss5("{-1 2 4} {2 -3 4} {} {}");
  PartialBeliefState pbs5;
  iss5 >> pbs5;

  std::istringstream iss6("{1 2 4} {3 4} {} {}");
  PartialBeliefState pbs6;
  iss6 >> pbs6;


  PartialBeliefState* bs1 = new PartialBeliefState(pbs1);
  PartialBeliefState* bs2 = new PartialBeliefState(pbs2);
  PartialBeliefState* bs3 = new PartialBeliefState(pbs3);
  PartialBeliefState* bs4 = new PartialBeliefState(pbs4);
  PartialBeliefState* bs5 = new PartialBeliefState(pbs5);
  PartialBeliefState* bs6 = new PartialBeliefState(pbs6);
  
  ModelSessionIdListPtr msl(new ModelSessionIdList);

  ModelSessionId m1(bs1, 1, 1);
  ModelSessionId m2(bs2, 2, 2);
  ModelSessionId m3(bs3, 3, 3);
  ModelSessionId m4(bs4, 4, 4);
  ModelSessionId m5(bs5, 5, 5);
  ModelSessionId m6(bs6, 6, 6);

  msl->push_back(m1);
  msl->push_back(m2);
  msl->push_back(m3);
  msl->push_back(m4);
  msl->push_back(m5);
  msl->push_back(m6);

  std::cerr << "bs1: " << bs1 << std::endl;
  std::cerr << "bs2: " << bs2 << std::endl;
  std::cerr << "bs3: " << bs3 << std::endl;
  std::cerr << "bs4: " << bs4 << std::endl;
  std::cerr << "bs5: " << bs5 << std::endl;
  std::cerr << "bs6: " << bs6 << std::endl;

  std::cerr << "After pushing in to the list:" << std::endl;
  for (ModelSessionIdList::iterator it = msl->begin(); it != msl->end(); ++it)
    {
      std::cerr << "it->partial_belief_state: " << it->partial_belief_state << std::endl;
    }

  PartialBeliefState pbs8 = *bs2;

  std::cerr << "Before removing duplication:" << std::endl
	    << *msl << std::endl;

  remove_duplication(msl);

  std::cerr << "After removing duplication:" << std::endl
	    << *msl << std::endl;

  std::cerr << "Old bs2 = " << pbs8 << std::endl;

  PartialBeliefState* bs8 = new PartialBeliefState(pbs8);

  std::cerr << "Real Old bs2 = " << bs2 << " " << *bs2 << std::endl;
  
  delete bs8;

  BOOST_CHECK_EQUAL(msl->size(), 3);
  
  // Now I release memory
  for (ModelSessionIdList::iterator it = msl->begin(); it != msl->end(); ++it)
    {
      //      PartialBeliefState*& bs = it->partial_belief_state;
      //      delete bs;
      //      bs = 0;
      
      std::cerr << "it->partial_belief_state: " << it->partial_belief_state << std::endl;
      delete it->partial_belief_state;
      it->partial_belief_state = 0;
    }

  /*
  if (bs1)
    {
      std::cerr << "bs1 still active: " << bs1 << ": " << *bs1 << std::endl;
    }

  if (bs2)
    {
      std::cerr << "bs2 still active: " << bs2 << ": " <<*bs2 << std::endl;
    }

  if (bs3)
    {
      std::cerr << "bs3 still active: " << bs3 << ": " <<*bs3 << std::endl;
    }

  if (bs4)
    {
      std::cerr << "bs4 still active: " << bs4 << ": " <<*bs4 << std::endl;
    }

  if (bs5)
    {
      std::cerr << "bs5 still active: " << bs5 << ": " <<*bs5 << std::endl;
    }

  if (bs6)
    {
      std::cerr << "bs6 still active: " << bs6 << ": " <<*bs6 << std::endl;
      }*/

}
