#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/BeliefCombination.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBeliefComparison"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;


BOOST_AUTO_TEST_CASE( testBeliefStateCombination )
{
  std::size_t system_size = 2;

  BeliefStateListPtr cs(new BeliefStateList);
  BeliefStateListPtr ct(new BeliefStateList);

  // create fresh BeliefState of length system_size.
  //
  // attention, we need to construct it with the empty BeliefSet, if
  // we call BeliefState(system_size, 0) we implicitly call it like
  // BeliefState(system_size, BeliefSet(0)) which gives us BeliefSets
  // that have 0 size and results in unpredictable behaviour later on

  BeliefStatePtr s1(new BeliefState(system_size, BeliefSet()));
  BeliefStatePtr s2(new BeliefState(system_size, BeliefSet()));
  BeliefStatePtr t1(new BeliefState(system_size, BeliefSet()));
  BeliefStatePtr t2(new BeliefState(system_size, BeliefSet()));


  //
  // S1 = ( 001011, 000001 )
  //
  BeliefState::iterator it = s1->begin();

  BeliefSet& s11 = *it;
  s11.set(1);
  s11.set(3);
  setEpsilon(s11);
  
  BeliefSet& s12 = *(++it);
  setEpsilon(s12);

  //
  // S2 = ( 000111, 000001 )
  //
  it = s2->begin();

  BeliefSet& s21 = *it;
  s21.set(1);
  s21.set(2);
  setEpsilon(s21);
  
  BeliefSet& s22 = *(++it);
  setEpsilon(s22);

  cs->push_back(s1);
  cs->push_back(s2);

  
  //
  // T1 = (001111, 000001)
  //
  it = t1->begin();

  BeliefSet& t11 = *it;
  t11.set(1);
  t11.set(2);
  t11.set(3);
  setEpsilon(t11);
  
  BeliefSet& t12 = *(++it);
  setEpsilon(t12);

  //
  // T2 = (000111, 000001)
  //
  it = t2->begin();

  BeliefSet& t21 = *it;
  t21.set(1);
  t21.set(2);
  setEpsilon(t21);
  
  BeliefSet& t22 = *(++it);
  setEpsilon(t22);
  
  ct->push_back(t1);
  ct->push_back(t2);

  BOOST_CHECK_EQUAL(cs->size(), 2);
  BOOST_CHECK_EQUAL(ct->size(), 2);

  BeliefStatePtr Vmax(new BeliefState(system_size, maxBeliefSet(32)));

  // now combine
  BeliefStateListPtr cu = combine(cs, ct, Vmax);

  BOOST_CHECK_EQUAL(cu->size(), 1); // only S2 matches with T2 on Vmax

  BeliefStatePtr Vmin(new BeliefState(system_size, BeliefSet()));

  // now combine
  cu = combine(cs, ct, Vmin);

  BOOST_CHECK_EQUAL(cu->size(), 4); // everything matches with everything on Vmin

  // == 111b == 0x07
  BeliefSet v7;
  v7.set(0);
  v7.set(1);
  v7.set(2);

  BeliefStatePtr V7(new BeliefState(system_size, v7));

  // now combine
  cu = combine(cs, ct, V7);
  
  BOOST_CHECK_EQUAL(cu->size(), 2); // S2 matches with T1,T2 on V7

  cu->sort(BeliefStateCmp());
  cu->unique(BeliefStateEq());

  BOOST_CHECK_EQUAL(cu->size(), 1); // only 1 left...
}
