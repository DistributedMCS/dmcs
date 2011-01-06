/* DMCS -- Distributed Nonmonotonic Multi-Context Systems.
 * Copyright (C) 2009, 2010 Minh Dao-Tran, Thomas Krennwallner
 * 
 * This file is part of DMCS.
 *
 *  DMCS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DMCS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DMCS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   LoopFormula.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Fri Jan 1 2010
 * 
 * @brief  
 * 
 * 
 */


#ifndef LOOP_FORMULA_H
#define LOOP_FORMULA_H

#include "mcs/Rule.h"
#include "mcs/Theory.h"

#include <iostream>
#include <vector>
#include <algorithm>


namespace dmcs {


template<typename AtomSetIterator>
struct PBodyShare
{
  bool
  operator() (const PositiveBody& pbody, AtomSetIterator abeg, AtomSetIterator aend) const
  {
    PositiveBody::const_iterator pit = std::find_first_of(pbody.begin(), pbody.end(), abeg, aend);
    return pit != pbody.end();
  }
};


template<typename AtomSetIterator>
struct HeadShare
{
  bool
  operator() (const Head& head, AtomSetIterator abeg, AtomSetIterator aend) const
  {
    Head::const_iterator hit = std::find_first_of(head.begin(), head.end(), abeg, aend);
    return hit != head.end();
  }
};


template<typename AtomSetIterator, typename RuleIterator>
std::vector<RuleIterator>
externalSupportRules(AtomSetIterator abeg, AtomSetIterator aend,
		     RuleIterator rbeg, RuleIterator rend)
{
  std::vector<RuleIterator> ret;

  for (RuleIterator it = rbeg; it != rend; ++it) 
    {
      PBodyShare<AtomSetIterator> pbs;
      if (pbs(getPositiveBody(*it), abeg, aend))
	{
	  continue;
	}
      
      HeadShare<AtomSetIterator> hs;
      if (!hs(getHead(*it), abeg, aend))
	{
          continue;
        }  
      
      ret.push_back(it);
    }
  
  return ret;
}


template<typename AtomSetIterator, typename RuleIterator>
std::vector<RuleIterator>
supportRules(AtomSetIterator abeg, AtomSetIterator aend,
	     RuleIterator rbeg, RuleIterator rend)
{
  std::vector<RuleIterator> ret;
  
  for (RuleIterator it = rbeg; it != rend; ++it) 
    {
      HeadShare<AtomSetIterator> hs;
      if (!hs(getHead(*it), abeg, aend))
	{
          continue;
        }  
      
      ret.push_back(it);
    }

  return ret;
}



struct EpsilonConjunction
{
  typedef std::list<int> Conjunction; // we need a list that allows sort() for set operations
  Conjunction posLiterals;
  Conjunction negLiterals;
};

template<typename AtomSetIterator, typename RuleIterator>
void
supportFormula(AtomSetIterator abeg, AtomSetIterator aend,
	       RuleIterator rbeg, RuleIterator rend,
	       std::list<EpsilonConjunction>& disj)
{
  for (RuleIterator it = rbeg; it != rend; ++it) 
    {
      EpsilonConjunction literals; // conjunction of literals
      std::back_insert_iterator<EpsilonConjunction::Conjunction> nlins(literals.negLiterals);

      const Head& head = getHead(**it);

      Head copyh;
      for(Head::const_iterator ith=head.begin();ith!=head.end();ith++)
          	  copyh.push_back(*ith);

      copyh.sort();

      std::set_difference(copyh.begin(), copyh.end(), abeg, aend, nlins);

      const NegativeBody& nbody = getNegativeBody(**it);
      std::copy(nbody.begin(), nbody.end(), nlins);
      // std::transform(nbody.begin(), nbody.end(), nlins, std::negate<Atom>());
      
      const PositiveBody& pbody = getPositiveBody(**it);
      std::back_insert_iterator<EpsilonConjunction::Conjunction> plins(literals.posLiterals);
      std::copy(pbody.begin(), pbody.end(), plins);

      disj.push_back(literals);
    }
}


///@todo TK: setIterator is an ugly name
template<typename setIterator>
ClausePtr
createNegatedConjunction(setIterator abeg, setIterator aend)
{
  ClausePtr conj(new Clause);

  // transform didnt work for some reason
  // std::transform(abeg, aend, conj->begin(), std::negate<Atom>());

  for(setIterator it = abeg; it != aend; ++it)
    {
      conj->push_back(-1* (*it) );
    }
  return conj;
}


inline ClausePtr
mergeTwoConjunctions(ClausePtr conj1, ClausePtr conj2)
{
  std::sort(conj1->begin(),conj1->end());
  std::sort(conj2->begin(),conj2->end());
  
  ClausePtr v(new Clause);
  std::back_insert_iterator<Clause> insertv(*v);
  std::set_union(conj1->begin(),conj1->end(),
		 conj2->begin(),conj2->end(),
		 insertv);
  return v;
}


template<typename AtomSetIterator, typename RuleIterator>
void
supportFormula(AtomSetIterator abeg, AtomSetIterator aend,
	       RuleIterator rbeg, RuleIterator rend,
	       TheoryPtr& formula)
{

  for (RuleIterator it = rbeg; it != rend; ++it) 
    {
      ClausePtr currentRule (new Clause);

      std::back_insert_iterator<Clause> jt (*currentRule);
      const PositiveBody& pbody = getPositiveBody(**it);
      std::copy(pbody.begin(), pbody.end(),jt);

      ///@todo rewrite the copying with negation in a better way
      ClausePtr currentRuleHeadPrime (new Clause);
      const Head& head = getHead(**it);

      Head copyh;
      for(Head::const_iterator ith=head.begin();ith!=head.end();ith++)
          	  copyh.push_back(*ith);

      copyh.sort();

      std::back_insert_iterator<Clause> nlins1(*currentRuleHeadPrime);

      std::set_difference(copyh.begin(), copyh.end(), abeg, aend, nlins1);
      currentRuleHeadPrime = createNegatedConjunction(currentRuleHeadPrime->begin(),
						      currentRuleHeadPrime->end());

      ClausePtr currentRuleNegBody (new Clause);
      const NegativeBody& nbody = getNegativeBody(**it);
      currentRuleNegBody = createNegatedConjunction(nbody.begin(), nbody.end());

      
      currentRule = mergeTwoConjunctions(currentRule, currentRuleNegBody);
      currentRule = mergeTwoConjunctions(currentRule, currentRuleHeadPrime);      

      add(formula,currentRule);
    }
}


} // namespace dmcs

#endif /* LOOPFORMULA_H */


// Local Variables:
// mode: C++
// End:
