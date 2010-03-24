#include <iostream>
#include "BeliefState.h"

using namespace dmcs;

std::size_t system_size = 2;
BeliefStatesPtr cs(new BeliefStates(system_size));
BeliefStatesPtr ct(new BeliefStates(system_size));

void create_belief_states()
{
  BeliefState s1;
  BeliefState s2;
  BeliefState t1;
  BeliefState t2;

  std::pair<int, char> p1(1, 'T');
  std::pair<int, char> p2(2, 'F');
  std::pair<int, char> p3(3, 'T');
  std::pair<int, char> p4(4, 'F');
  std::pair<int, char> p5(5, 'F');

  std::pair<int, char> q1(1, 'T');
  std::pair<int, char> q2(2, 'T');
  std::pair<int, char> q3(3, 'F');
  std::pair<int, char> q4(4, 'F');
  std::pair<int, char> q5(5, 'F');
  

  std::pair<int, char> r1(1, 'T');
  std::pair<int, char> r2(2, 'T');
  std::pair<int, char> r3(3, 'T');
  std::pair<int, char> r4(4, 'F');
  std::pair<int, char> r5(5, 'F');

  std::pair<int, char> u1(1, 'T');
  std::pair<int, char> u2(2, 'T');
  std::pair<int, char> u3(3, 'T');
  std::pair<int, char> u4(4, 'F');
  std::pair<int, char> u5(5, 'F');
  
  BeliefSet s11;
  s11.insert(p1);
  s11.insert(p2);
  s11.insert(p3);
  
  BeliefSet s12;
  s12.insert(p4);
  s12.insert(p5);
  
  std::pair<int, BeliefSet> ps11(1, s11);
  std::pair<int, BeliefSet> ps12(2, s12);
  
  s1.mapping.insert(ps11);
  s1.mapping.insert(ps12);
  
  cs->push_back(s1);

  //-------------------------------------

  BeliefSet s21;
  s21.insert(q1);
  s21.insert(q2);
  s21.insert(q3);
  
  BeliefSet s22;
  s22.insert(q4);
  s22.insert(q5);

  std::pair<int, BeliefSet> ps21(1, s21);
  std::pair<int, BeliefSet> ps22(2, s22);
  
  s2.mapping.insert(ps21);
  s2.mapping.insert(ps22);
  
  cs->push_back(s2);

  //-------------------------------------

  BeliefSet t11;
  t11.insert(r1);
  t11.insert(r2);
  t11.insert(r3);
  
  BeliefSet t12;
  t12.insert(q4);
  t12.insert(q5);

  std::pair<int, BeliefSet> pt11(1, t11);
  std::pair<int, BeliefSet> pt12(2, t12);
  
  t1.mapping.insert(pt11);
  //t1.mapping.insert(pt12);
  
  ct->push_back(t1);

  //-------------------------------------

  BeliefSet t21;
  t21.insert(u1);
  t21.insert(u2);
  t21.insert(u3);
  
  BeliefSet t22;
  t22.insert(u4);
  t22.insert(u5);

  std::pair<int, BeliefSet> pt21(1, t21);
  std::pair<int, BeliefSet> pt22(2, t22);
  
  t2.mapping.insert(pt21);
  t2.mapping.insert(pt22);
  
  ct->push_back(t2);
}

void print_belief_states(BeliefStatesPtr& belief_states)
{
  for (std::size_t i = 0; i < belief_states->size(); ++i)
    {
      std::cerr << "+Belief state number " << i+1 << ": ";
      std::cerr << "( ";
      for (BeliefState::Map::const_iterator it1 = belief_states->at(i).mapping.begin();
	   it1 != belief_states->at(i).mapping.end(); ++ it1)
	{
	  //std::cerr << "-Belief set number " << it1->first << std::endl;
	  std::cerr << "{";
	  for (BeliefSet::const_iterator it2 = it1->second.begin(); it2 != it1->second.end(); ++it2)
	    {
	      std::cerr << it2->first << it2->second << ",";
	    }
	  std::cerr << "}, ";
	}
      std::cerr << ")" << std::endl;
    }  
}

int main()
{
    create_belief_states();

    std::cerr << "CS:" << std::endl;
    print_belief_states(cs);
    
    std::cerr << "CT:" << std::endl;
    print_belief_states(ct);

    int i = 0;
    for (BeliefStates::iterator cs1 = cs->begin(); cs1 != cs->end(); ++cs1)
    {
	++i;
	int j = 0;
	for (BeliefStates::iterator ct1 = ct->begin(); ct1 != ct->end(); ct1++)
	{
	    ++j;
	    if (*cs1 < *ct1)
	    {
		std::cerr << "cs[" << i << "] < ct[" << j << "]" << std::endl;	
	    }
	    else
	    {
		std::cerr << "cs[" << i << "] not < ct[" << j << "]" << std::endl;
	    }
	}
    }
}
