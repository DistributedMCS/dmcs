#include "BeliefState.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBeliefComparison"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;


BOOST_AUTO_TEST_CASE( testBeliefStates )
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

  BeliefStateList::const_iterator s_it = cs->begin();
  BeliefStateList::const_iterator t_it = ct->begin();

  // compare S1 to T1
  const BeliefStatePtr& my_s1 = *s_it;
  const BeliefStatePtr& my_t1 = *t_it;

  BOOST_CHECK_EQUAL(*s_it, s1);
  BOOST_CHECK_EQUAL(*t_it, t1);

  BOOST_CHECK_EQUAL(my_s1->size(), system_size);
  BOOST_CHECK_EQUAL(my_t1->size(), system_size);

  BOOST_CHECK_NE( my_s1->at(0), my_t1->at(0) );
  BOOST_CHECK_EQUAL( (*my_s1)[1], (*my_t1)[1] );

  // vector::at() bails, vector::operator[] doesn't
  //my_s1->at(2);

  // compare S2 to T2

  ++s_it;
  ++t_it;

  BOOST_CHECK_EQUAL(*s_it, s2);
  BOOST_CHECK_EQUAL(*t_it, t2);

  const BeliefState& my_s2 = **s_it;
  const BeliefState& my_t2 = **t_it;

  ///@todo GRRRRR BOOST_CHECK_BITWISE_EQUAL does not work with shared_ptr

  BOOST_CHECK_EQUAL(my_s2.size(), system_size);
  BOOST_CHECK_EQUAL(my_t2.size(), system_size);

  BOOST_CHECK_EQUAL( my_s2[0], my_t2[0] );
  BOOST_CHECK_EQUAL( my_s2[1], my_t2[1] );
}




BOOST_AUTO_TEST_CASE( testBeliefStateSorting )
{
  std::size_t system_size = 3;

  BeliefStateListPtr cs(new BeliefStateList);

  BeliefStatePtr s1(new BeliefState(system_size, 0));
  BeliefStatePtr s2(new BeliefState(system_size, 0));
  BeliefStatePtr s3(new BeliefState(system_size, 0));


  //
  // S1 = ( 001011, 000001, 000000 )
  //
  BeliefState::iterator it = s1->begin();

  BeliefSet& s11 = *it;
  s11.set(1);
  s11.set(3);
  setEpsilon(s11);
  
  BeliefSet& s12 = *(++it);
  setEpsilon(s12);


  //
  // S2 = ( 001011, 000001, 000000 )
  //
  it = s2->begin();

  BeliefSet& s21 = *it;
  s21.set(1);
  s21.set(3);
  setEpsilon(s21);
  
  BeliefSet& s22 = *(++it);
  setEpsilon(s22);

 
  //
  // S3 = ( 000011, 000001, 000000 )
  //
  it = s3->begin();

  BeliefSet& s31 = *it;
  s31.set(1);
  setEpsilon(s31);
  
  BeliefSet& s32 = *(++it);
  setEpsilon(s32);

  cs->push_back(s1);
  cs->push_back(s2);
  cs->push_back(s3);
  

  BOOST_CHECK_EQUAL(cs->size(), 3);

  BeliefStateList::const_iterator s_it = cs->begin();

  BOOST_CHECK_EQUAL(*s_it, s1);
  BOOST_CHECK_EQUAL(*(++s_it), s2);
  BOOST_CHECK_EQUAL(*(++s_it), s3);

  cs->sort(BeliefStateCmp());

  s_it = cs->begin();

  BOOST_CHECK_EQUAL(*s_it, s3);
  BOOST_CHECK_EQUAL(*(++s_it), s1);
  BOOST_CHECK_EQUAL(*(++s_it), s2);

  cs->unique(BeliefStateEq());

  BOOST_CHECK_EQUAL(cs->size(), 2);

  s_it = cs->begin();

  BOOST_CHECK_EQUAL(*s_it, s3);
  BOOST_CHECK_EQUAL(*(++s_it), s1);
}
