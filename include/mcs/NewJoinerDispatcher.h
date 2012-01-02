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
 * @file   NewJoinerDispatcher.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  2 20:53:10 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_JOINER_DISPATCHER_H
#define NEW_JOINER_DISPATCHER_H

#include "mcs/NewBaseDispatcher.h"
#include "mcs/JoinIn.h"
#include "mcs/QueryID.h"
#include "mcs/ReturnedBeliefState.h"

namespace dmcs {

class NewJoinerDispatcher : public NewBaseDispatcher
{
public:
  NewJoinerDispatcher(NewConcurrentMessageDispatcherPtr& md)
    : NewBaseDispatcher(md)
  { }

  void
  operator()()
  {
    while (1)
      {
	int timeout = 0;
	NewJoinIn* notification = cmd->receive<NewJoinIn>(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, timeout);
	std::size_t neighbor_offset = notification->neighbor_offset;
	std::size_t no_belief_state = notification->no_belief_state;
	
	delete notification;
	notification = 0;

	for (std::size_t i = 0; i < no_belief_state; ++i)
	  {
	    ReturnedBeliefState* rbs = cmd->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_offset, timeout);
	    set_neighbor_offset(rbs->query_id, neighbor_offset);
	    
	    std::size_t ctx_id = ctxid_from_qid(rbs->query_id);
	    std::size_t offset = get_offset(ctx_id);
	    
	    cmd->send(NewConcurrentMessageDispatcher::JOIN_IN_MQ, offset, rbs, timeout);
	  }
      }
  }
};

} // namespace dmcs

#endif // NEW_JOINER_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
