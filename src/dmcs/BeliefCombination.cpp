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
 * @file   BeliefCombination.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Nov  17 16:53:24 2009
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/BeliefCombination.h"

#include <algorithm>

#if defined(DEBUG)
#include <bitset>
#endif

namespace dmcs {

void
printBeliefStateNicely(std::ostream& os, const BeliefStatePtr& b_ptr, 
		       const BeliefStatePtr& V, const QueryPlanPtr& query_plan)
{
  const BeliefState& belief_sets = *b_ptr;
  const BeliefState& mask = *V;

  assert(belief_sets.size() == mask.size());

  const std::size_t n = belief_sets.size();

  BeliefState::const_iterator bt = belief_sets.begin();
  BeliefState::const_iterator vt = mask.begin();

  for(std::size_t j = 1; j <= n; ++bt, ++vt, ++j)
     {
       const BeliefSet b = *bt;
       const BeliefSet v = *vt;
       os << "{";
       const Signature& sig = query_plan->getSignature(j);
       const SignatureByLocal& sig_local = boost::get<Tag::Local>(sig);
       std::size_t sig_size = sig.size();
       
       if (!isEpsilon(b))
 	{
	  for (std::size_t i = 1; // ignore epsilon bit
	       i <= sig_size; ++i)
 	    {
 	      if (testBeliefSet(v, i))
 		{
		  SignatureByLocal::const_iterator local_it = sig_local.find(i);

		  if (testBeliefSet(b, i))
		    {
		      os  << local_it->sym << " ";
		    }
		  else
		    {
		      os  << "-" << local_it->sym << " ";	
		    }
		}
	    }
	}
      os << "}, ";
    }
  os << std::endl;
}



void
printBeliefStatesNicely(std::ostream& os, const BeliefStateListPtr& bs_ptr, 
			const BeliefStatePtr& V, const QueryPlanPtr& query_plan)
{
  for(BeliefStateList::const_iterator it = bs_ptr->begin();
      it != bs_ptr->end();
      ++it)
    {
      printBeliefStateNicely(os, *it, V, query_plan);
    }
}


/** 
 * Update s by t. Just approriate for genTest.
 * 
 * @param s 
 * @param t 
 *
 */
void
update(BeliefStatePtr& s, const BeliefStatePtr& t)
{
  assert (s->size() == t->size());

  BeliefState::iterator s_it = s->begin();
  BeliefState::const_iterator t_it = t->begin();

  for (; s_it != s->end(); ++s_it, ++t_it)
    {
      *s_it |= *t_it;
    }
}


/** 
 * Try to consistently join s and t wrt. V to u.
 * 
 * @param s 
 * @param t 
 * @param u 
 * @param V 
 * 
 * @return true if join is consistent, false otw.
 */
bool
combine(const BeliefStatePtr& s, const BeliefStatePtr& t, BeliefStatePtr& u, 
	const BeliefStatePtr& V)
{
  // all belief states must have identical size
  assert((s->size() == t->size()) &&
	 (t->size() == u->size()) &&
	 (u->size() == V->size()));
  assert(V->size() > 0);

  BeliefState::const_iterator s_it = s->begin();
  BeliefState::const_iterator t_it = t->begin();
  BeliefState::const_iterator v_it = V->begin();
  BeliefState::iterator u_it = u->begin();

  // walk through the belief sets
  for (; s_it != s->end(); ++s_it, ++t_it, ++u_it, ++v_it)
    {
#if 0
      std::bitset<sizeof(BeliefSet)*8> sbs = *s_it;
      std::bitset<sizeof(BeliefSet)*8> tbs = *t_it;
      std::bitset<sizeof(BeliefSet)*8> vbs = *v_it;
      std::cerr << "  Combining S = " << sbs << std::endl
		<< "  with      T = " << tbs << std::endl 
		<< "  on        V = " << vbs << std::endl;
#endif

      if (isEpsilon(*s_it))
	{
	  *u_it = *t_it;
	}
      else if (isEpsilon(*t_it))
	{
	  *u_it = *s_it;
	}
      else if ((*s_it & *v_it) == (*t_it & *v_it))
	{
	  *u_it = *s_it & *v_it;
	}
      else 
	{
#if 0
	  std::cerr << " Upps. MISMATCH at position" << std::endl;
#endif
	  return false;
	}

    } // for
  
#if 0
  std::cerr << "Combination suceeded. u = " << u << std::endl;
#endif

  return true; // consistent
}


	      
/** 
 * 
 * 
 * @param cs 
 * @param ct 
 * @param V
 * 
 * @return 
 */
BeliefStateListPtr 
combine(const BeliefStateListPtr& cs, const BeliefStateListPtr& ct, 
	const BeliefStatePtr& V)
{
  // some sanity checks
  assert(V->size() > 0);

#if 0
  std::cerr << "Combining " << cs << std::endl << " and " ;
  std::cerr << std::endl << ct << std::endl;
#endif

  const std::size_t n = V->size();

  BeliefStateListPtr cu(new BeliefStateList);
  
  for (BeliefStateList::const_iterator s_it = cs->begin(); s_it != cs->end(); ++s_it)
    {
      for (BeliefStateList::const_iterator t_it = ct->begin(); t_it != ct->end(); ++t_it)
	{
#if 0
	  std::cerr << "Combining " << *s_it << " and " << *t_it << std::endl;
	  std::cerr << "Combining: " << std::endl;
	  printBeliefStateNicely(std::cerr, s, V, query_plan);
	  std::cerr << "with: " << std::endl;
	  printBeliefStateNicely(std::cerr, t, V, query_plan);
#endif
	  
	  BeliefStatePtr u(new BeliefState(n, 0)); // start with empty belief state

	  if (combine(*s_it, *t_it, u, V))
	    {
	      cu->push_back(u);
	    }	    
	}
    }
  
  return cu;
}



void
project_to(const BeliefStateListPtr& cs, const BeliefStatePtr& v, BeliefStateListPtr& cu)
{
 #ifdef DEBUG
   std::cerr << "Projecting " << std::endl << *cs << std::endl;
   std::cerr << "to         " << std::endl << v << std::endl;
 #endif

   for (BeliefStateList::const_iterator it = cs->begin(); it != cs->end(); ++it)
     {
       const BeliefStatePtr& is = *it;

       BeliefStatePtr u(new BeliefState(is->size(), 0)); // create empty belief state
       BeliefState::iterator ut = u->begin();

       BeliefState::const_iterator vt = v->begin();

       for (BeliefState::const_iterator ct = is->begin();
	    ct != is->end();
	    ++ct, ++vt, ++ut)
 	{
 	  *ut = *ct & *vt;
 	}

       cu->push_back(u);

       ///@todo TK: we could also get rid off duplicates outside the
       ///loop, this needs some testing

       ///experiments show that projection sometime takes a lot of
       ///time due to sort and unique.

       ///@todo: use set for internal
       ///accumulation and return a list another possibility is to use
       ///multi-index-container
       cu->sort(BeliefStateCmp());
       cu->unique(BeliefStateEq());
     }
}

} // namespace dmcs
