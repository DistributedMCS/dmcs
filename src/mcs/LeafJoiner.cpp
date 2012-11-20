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
  * @file   LeafJoiner.cpp
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Tue Nov  20 11:13:26 2012
  * 
  * @brief  
  * 
  * 
  */

#include "mcs/LeafJoiner.h"

namespace dmcs {



LeafJoiner::LeafJoiner(NewNeighborVecPtr n)
  : BaseJoiner(n)
{ }



ReturnedBeliefState*
LeafJoiner::first_join(std::size_t query_id,
		       const NewHistory& history,
		       NewConcurrentMessageDispatcherPtr md,
		       NewJoinerDispatcherPtr jd)
{
  NewBeliefState* input = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
					     BeliefStateOffset::instance()->SIZE_BS());
  
  ReturnedBeliefState* rbs = new ReturnedBeliefState(input, query_id);
  return rbs;
}



ReturnedBeliefState*
LeafJoiner::next_join(std::size_t query_id,
		      const NewHistory& history,
		      NewConcurrentMessageDispatcherPtr md,
		      NewJoinerDispatcherPtr jd)
{
  ReturnedBeliefState* rbs = new ReturnedBeliefState(NULL, query_id);
  return rbs;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
