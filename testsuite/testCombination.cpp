#include "BeliefCombination.h"

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

  BeliefStatePtr s1(new BeliefState(system_size, 0));
  BeliefStatePtr s2(new BeliefState(system_size, 0));
  BeliefStatePtr t1(new BeliefState(system_size, 0));
  BeliefStatePtr t2(new BeliefState(system_size, 0));


  //
  // S1 = ( 001011, 000001 )
  //
  BeliefState::iterator it = s1->begin();

  BeliefSet& s11 = *it;
  s11 = setBeliefSet(s11, 1);
  s11 = setBeliefSet(s11, 3);
  s11 = setEpsilon(s11);
  
  BeliefSet& s12 = *(++it);
  s12 = setEpsilon(s12);

  //
  // S2 = ( 000111, 000001 )
  //
  it = s2->begin();

  BeliefSet& s21 = *it;
  s21 = setBeliefSet(s21, 1);
  s21 = setBeliefSet(s21, 2);
  s21 = setEpsilon(s21);
  
  BeliefSet& s22 = *(++it);
  s22 = setEpsilon(s22);

  cs->push_back(s1);
  cs->push_back(s2);

  
  //
  // T1 = (001111, 000001)
  //
  it = t1->begin();

  BeliefSet& t11 = *it;
  t11 = setBeliefSet(t11, 1);
  t11 = setBeliefSet(t11, 2);
  t11 = setBeliefSet(t11, 3);
  t11 = setEpsilon(t11);
  
  BeliefSet& t12 = *(++it);
  t12 = setEpsilon(t12);

  //
  // T2 = (000111, 000001)
  //
  it = t2->begin();

  BeliefSet& t21 = *it;
  t21 = setBeliefSet(t21, 1);
  t21 = setBeliefSet(t21, 2);
  t21 = setEpsilon(t21);
  
  BeliefSet& t22 = *(++it);
  t22 = setEpsilon(t22);
  
  ct->push_back(t1);
  ct->push_back(t2);

  BOOST_CHECK_EQUAL(cs->size(), 2);
  BOOST_CHECK_EQUAL(ct->size(), 2);

  BeliefStatePtr Vmax(new BeliefState(system_size, maxBeliefSet()));

  // now combine
  BeliefStateListPtr cu = combine(cs, ct, Vmax);

  BOOST_CHECK_EQUAL(cu->size(), 1); // only S2 matches with T2 on Vmax

  BeliefStatePtr Vmin(new BeliefState(system_size, 0));

  // now combine
  cu = combine(cs, ct, Vmin);

  BOOST_CHECK_EQUAL(cu->size(), 4); // everything matches with everything on Vmin


  BeliefStatePtr V07(new BeliefState(system_size, 0x07));

  // now combine
  cu = combine(cs, ct, V07);
  
  BOOST_CHECK_EQUAL(cu->size(), 2); // S2 matches with T1,T2 on V07

  cu->sort(BeliefStateCmp());
  cu->unique(BeliefStateEq());

  BOOST_CHECK_EQUAL(cu->size(), 1); // only 1 left...
}
