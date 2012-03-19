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

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include "mcs/NewBaseDispatcher.h"
#include "mcs/JoinIn.h"
#include "mcs/QueryID.h"
#include "mcs/ReturnedBeliefState.h"

namespace dmcs {

class NewJoinerDispatcher : public NewBaseDispatcher
{
public:
  NewJoinerDispatcher()
  { }

  void
  startup(NewConcurrentMessageDispatcherPtr md)
  {
    while (1)
      {
	int timeout = 0;
	NewJoinIn* notification = md->receive<NewJoinIn>(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, timeout);
	std::size_t neighbor_offset = notification->neighbor_offset;
	std::size_t no_belief_state = notification->no_belief_state;
	
	delete notification;
	notification = 0;

	for (std::size_t i = 0; i < no_belief_state; ++i)
	  {
	    ReturnedBeliefState* rbs = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_offset, timeout);
	    
	    std::size_t old_qid = rbs->qid;
	    set_neighbor_offset(rbs->qid, neighbor_offset);
	    std::size_t offset = get_offset(old_qid);
	    
	    md->send(NewConcurrentMessageDispatcher::JOIN_IN_MQ, offset, rbs, timeout);
	  }
	boost::this_thread::interruption_point();
      }
  }
};

typedef boost::shared_ptr<NewJoinerDispatcher> NewJoinerDispatcherPtr;

struct NewJoinerDispatcherWrapper
{
  void
  operator()(NewJoinerDispatcherPtr jd,
	     NewConcurrentMessageDispatcherPtr md)
  {
    jd->startup(md);
  }
};

} // namespace dmcs

#endif // NEW_JOINER_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
