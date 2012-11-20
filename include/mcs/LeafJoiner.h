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
  * @file   LeafJoiner.h
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Tue Nov  20 11:04:06 2012
  * 
  * @brief  
  * 
  * 
  */

#ifndef __LEAF_JOINER_H__
#define __LEAF_JOINER_H__

#include "mcs/BaseJoiner.h"
#include "mcs/BeliefStateOffset.h"

namespace dmcs {

class LeafJoiner : public BaseJoiner
{
public:
  LeafJoiner(NewNeighborVecPtr n);

  ReturnedBeliefState*
  first_join(std::size_t query_id,
	     const NewHistory& history,
	     NewConcurrentMessageDispatcherPtr md = NewConcurrentMessageDispatcherPtr(),
	     NewJoinerDispatcherPtr jd = NewJoinerDispatcherPtr());


  ReturnedBeliefState*
  next_join(std::size_t query_id,
	    const NewHistory& history,
	    NewConcurrentMessageDispatcherPtr md = NewConcurrentMessageDispatcherPtr(),
	    NewJoinerDispatcherPtr jd = NewJoinerDispatcherPtr());
};

typedef boost::shared_ptr<LeafJoiner> LeafJoinerPtr;

} // namespace dmcs

#endif // __LEAF_JOINER_H__


// Local Variables:
// mode: C++
// End:
