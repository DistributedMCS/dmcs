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
 * @file   Debug.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Thu Jan  6 10:43:21 2011
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/Debug.h"

#include <algorithm>
#include <iostream>

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


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
