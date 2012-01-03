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
  * @file   BaseJoiner.cpp
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Tue Jan  3 10:33:18 2012
  * 
  * @brief  
  * 
  * 
  */

#include "mcs/BaseJoiner.h"

namespace dmcs {

BaseJoiner::BaseJoiner(std::size_t c, std::size_t n,
		       NewConcurrentMessageDispatcherPtr m,
		       NewJoinerDispatcherPtr jd)
  : ctx_offset(c),
    no_neighbors(n),
    md(m),
    joiner_dispatcher(jd)
{
  init();
}



void
BaseJoiner::init()
{
  // set up a package of empty BeliefStates
  for (std::size_t i = 0; i < no_neighbors; ++i)
    {
      NewBeliefStateVecPtr bsv(new NewBeliefStateVec);
      input_belief_states.push_back(bsv);
    }
}



void
BaseJoiner::cleanup_input(std::size_t noff)
{
  NewBeliefStateVecPtr& bsv = input_belief_states[noff];
  for (NewBeliefStateVec::iterator it = bsv->begin(); it != bsv->end(); ++it)
    {
      assert (*it);
      delete *it;
      *it = 0;
    }
  
  bsv->clear();
}



void
BaseJoiner::reset()
{
  // clean up results
  for (ReturnedBeliefStateList::iterator it = joined_results.begin();
       it != joined_results.end(); ++it)
    {
      ReturnedBeliefState* rbs = *it;
      NewBeliefState* bs = rbs->belief_state;
      delete bs;
      bs = 0;
    }
  joined_results.clear();

  // clean up input
  for (std::size_t i = 0; i < no_neighbors; ++i)
    {
      cleanup_input(i);
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
