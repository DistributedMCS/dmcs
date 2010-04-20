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

#include "BeliefCombination.h"

#include <algorithm>



namespace dmcs {

void
printBeliefStateNicely(std::ostream& os, const BeliefStatePtr& b_ptr, 
		       const BeliefStatePtr& V, const QueryPlanPtr& query_plan)
{
  const BeliefSets belief_sets = b_ptr.belief_state_ptr->belief_state;
  const BeliefSets mask = V.belief_state_ptr->belief_state;

  assert(belief_sets.size() == mask.size());

  const std::size_t n = belief_sets.size();

  BeliefSets::const_iterator bt = belief_sets.begin();
  BeliefSets::const_iterator vt = mask.begin();
  
  std::size_t j = 1;
  for(; j <= n; ++bt, ++vt, ++j)
    {
      const BeliefSet b = *bt;
      const BeliefSet v = *vt;
      os << "{";
      const Signature& sig = query_plan->getSignature(j);
      const SignatureByLocal& sig_local = boost::get<Tag::Local>(sig);
      std::size_t sig_size = sig.size();
      
      if (!isEpsilon(b))
	{
	  std::size_t i = 1; // ignore epsilon bit	      
	  for (; i <= sig_size ; ++i)
	    {
	      if (v & (1 << i))
		{
		  SignatureByLocal::const_iterator local_it = sig_local.find(i);
		  if (b & (1 << i))
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
printBeliefStatesNicely(std::ostream& os, const BeliefStatesPtr& bs_ptr, 
			const BeliefStatePtr& V, const QueryPlanPtr& query_plan)
{
  BeliefStates::BeliefStateSet::const_iterator it = bs_ptr.belief_states_ptr->belief_states.begin();
  std::size_t i = 0;
  for(; it != bs_ptr.belief_states_ptr->belief_states.end(); ++it, ++i)
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
  int n = s.belief_state_ptr->belief_state.size();

  for (int i = 0; i < n; ++i)
    {
      s.belief_state_ptr->belief_state[i] |= t.belief_state_ptr->belief_state[i];
    }
}


/** 
 * Try to consistently join s and t to u.
 * 
 * @param s 
 * @param t 
 * @param u 
 * 
 * @return true if join is consistent, false otw.
 */
bool
combine(const BeliefStatePtr& s, const BeliefStatePtr& t, BeliefStatePtr& u, 
	const BeliefStatePtr& V)
{

  //  const BeliefStatePtr& V = query_plan->getGlobalV();

  //#ifdef DEBUG
  //  std::cerr << "Combining: " << std::endl;
  //  printBeliefStateNicely(std::cerr, s, V, query_plan);
  //  std::cerr << "with: " << std::endl;
  //  printBeliefStateNicely(std::cerr, t, V, query_plan);
  //#endif

  const BeliefSets& s_state = s.belief_state_ptr->belief_state;
  const BeliefSets& t_state = t.belief_state_ptr->belief_state;
  const BeliefSets& v_state = V.belief_state_ptr->belief_state;
  BeliefSets& u_state = u.belief_state_ptr->belief_state;

  assert((s_state.size() == t_state.size()) &&
	 (t_state.size() == u_state.size()));

  const std::size_t n = s_state.size(); // system size

  BeliefSets::const_iterator s_it = s_state.begin();
  BeliefSets::const_iterator t_it = t_state.begin();
  BeliefSets::const_iterator v_it = v_state.begin();
  BeliefSets::iterator u_it = u_state.begin();

  // walk through the belief sets
  for (std::size_t i = 0; i < n; ++s_it, ++t_it, ++u_it, ++v_it, ++i)
    {
#ifdef DEBUG
      std::cerr << "  Combining [" << i << "]: " << *s_it << " and " << *t_it << std::endl;
#endif
      if (isEpsilon(*s_it))
	{
	  *u_it = *t_it;
	}
      else if (isEpsilon(*t_it))
	{
	  *u_it = *s_it;
	}
      else if ((*s_it & (*v_it)) == (*t_it & (*v_it)))
	{
	  *u_it = *s_it & (*v_it);
	}
      else 
	{
#ifdef DEBUG
	  std::cerr << " Upps. MISMATCH at position" << std::endl;
#endif
	  return false;
	}

    } // for
  
#ifdef DEBUG
  std::cerr << "Combination suceeded. u = " << u << std::endl;
#endif

  return true; // consistent
}


	      
/** 
 * 
 * 
 * @param cs 
 * @param ct 
 * 
 * @return 
 */
BeliefStatesPtr 
combine(const BeliefStatesPtr& cs, const BeliefStatesPtr& ct, 
	const BeliefStatePtr& V)
{
  ///@todo we need a V here as a parameter and do not assume that V stems from the query_plan

  assert(cs.belief_states_ptr->system_size == ct.belief_states_ptr->system_size);
  assert((cs.belief_states_ptr->system_size > 0) && 
	 (ct.belief_states_ptr->system_size > 0));

  const BeliefStates::BeliefStateSet& s_set = cs.belief_states_ptr->belief_states;
  const BeliefStates::BeliefStateSet& t_set = ct.belief_states_ptr->belief_states;

  const std::size_t n = cs.belief_states_ptr->system_size;

  BeliefStatesPtr cu(new BeliefStates(n)); // start with empty BeliefStates
  BeliefStates::BeliefStateSet& u_set = cu.belief_states_ptr->belief_states;

#ifdef DEBUG
  std::cerr << "Combining " << cs << std::endl << " and " ;
  std::cerr << std::endl << ct << std::endl;
#endif
  
  for (BeliefStates::BeliefStateSet::const_iterator s_it = s_set.begin(); s_it != s_set.end(); ++s_it)
    {
      for (BeliefStates::BeliefStateSet::const_iterator t_it = t_set.begin(); t_it != t_set.end(); ++t_it)
	{
#ifdef DEBUG
	  std::cerr << "Combining " << *s_it << " and " << *t_it << std::endl;
#endif
	  
	  BeliefStatePtr u(new BeliefState(n));

	  if (combine(*s_it, *t_it, u, V))
	    {
	      u_set.insert(u);
	    }	    
	}
    }
  
  return cu;
}



void
project_to(const BeliefStatesPtr& cs, const BeliefStatePtr& v, BeliefStatesPtr& cu)
{
 #ifdef DEBUG
   std::cerr << "Projecting " << std::endl << cs << std::endl;
   std::cerr << "to         " << std::endl << v << std::endl;
 #endif

   const BeliefStates::BeliefStateSet& css = cs.belief_states_ptr->belief_states;

   for (BeliefStates::BeliefStateSet::const_iterator it = css.begin(); it != css.end(); ++it)
     {
       const BeliefSets& is = it->belief_state_ptr->belief_state;

       BeliefStatePtr u(new BeliefState(is.size()));

       BeliefSets::iterator ut = u.belief_state_ptr->belief_state.begin();
       BeliefSets::const_iterator vt = v.belief_state_ptr->belief_state.begin();

       for (BeliefSets::const_iterator ct = is.begin();
	    ct != is.end();
	    ++ct, ++vt, ++ut)
 	{
 	  *ut = (*ct) & (*vt);
 	}

       cu.belief_states_ptr->belief_states.insert(u);
     }

 #ifdef DEBUG
   //std::cerr << "Got " << std::endl << cu << std::endl;
 #endif
}

} // namespace dmcs
