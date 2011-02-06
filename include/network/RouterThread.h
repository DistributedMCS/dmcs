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
 * @file   RouterThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  14 19:17:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ROUTER_THREAD_H
#define ROUTER_THREAD_H

#include "dmcs/ConflictNotification.h"
#include "dmcs/UnsatNotification.h"
#include "network/ConcurrentMessageQueueHelper.h"

#include <boost/thread.hpp>

namespace dmcs {

class RouterThread
{
private:
  std::size_t port;

public:
  RouterThread(std::size_t p)
    : port(p)
  { }


  virtual
  ~RouterThread()
  {
    DMCS_LOG_TRACE(port << ": Cheerio!");
  }


  void
  operator()(ConcurrentMessageQueue* sat_router_notif,
	     ConcurrentMessageQueueVec* router_neighbors_notif)
  {
    while (1)
      {
	// wait for any conflict from the local solver
	UnsatNotification* un = 0;
	void *ptr         = static_cast<void*>(&un);
	unsigned int p    = 0;
	std::size_t recvd = 0;

	DMCS_LOG_TRACE(port << ": Waiting for notification from SAT solver.");

	sat_router_notif->receive(ptr, sizeof(un), recvd, p);

	if (ptr && un)
	  {
	    if (un->type == UnsatNotification::SHUTDOWN)
	      {
#if 0
		ConcurrentMessageQueueVec::iterator beg = router_neighbors_notif->begin();
		ConcurrentMessageQueueVec::iterator end = router_neighbors_notif->end();

		DMCS_LOG_TRACE(port << ": SHUTDOWN requested, informing " << std::distance(beg, end) << " neighbors.");

		// inform all neighbors about the shutdown
		for (ConcurrentMessageQueueVec::iterator it = beg; it != end; ++it)
		  {
		    std::size_t prio = 0;

		    ConflictNotification* ow_neighbor =
		      (ConflictNotification*) overwrite_send(it->get(), &cn, sizeof(cn), prio);

		    DMCS_LOG_TRACE(port << ": Sent SHUTDOWN to neighbor " << std::distance(it, end));

		    if (ow_neighbor)
		      {
			delete ow_neighbor;
			ow_neighbor = 0;
		      }
		  }
#endif //0

		DMCS_LOG_TRACE(port << ": SHUTDOWN requested");

		return;
	      }
	    else
	      {
		// forward request

		ConflictVec2p* new_conflicts    = un->all_new_conflicts;
		PartialBeliefState* partial_ass = un->partial_ass;
		Decisionlevel* decision         = un->decision;
		std::size_t session_id          = un->session_id;

		ConflictVec2p::const_iterator ct = new_conflicts->begin();

		ConcurrentMessageQueueVec::iterator beg = router_neighbors_notif->begin();
		ConcurrentMessageQueueVec::iterator end = router_neighbors_notif->end();

		for (ConcurrentMessageQueueVec::iterator it = beg; it != end; ++it, ++ct)
		  {
		    ConflictVec* c_vec = *ct;
		    ConflictNotification* cn = new ConflictNotification(c_vec, partial_ass, decision, session_id);
		    std::size_t prio = 0;

		    DMCS_LOG_TRACE(port << ": Send restart request to noff = " << std::distance(beg, it) 
				   << ". session_id = " << session_id
				   << ". conflicts = " << *c_vec 
				   << ". partial_ass = " << *partial_ass
				   << ". decision = " << *decision);

		    ConflictNotification* ow_neighbor =
		      (ConflictNotification*) overwrite_send(it->get(), &cn, sizeof(cn), prio);

		    if (ow_neighbor)
		      {
			delete ow_neighbor;
			ow_neighbor = 0;
		      }
		  }
	      }
	  }
	else
	  {
	    DMCS_LOG_FATAL(port << ": Got null message: " << ptr << " " << un);
	    assert (ptr != 0 && un != 0);
	  }
      }
  }

};

typedef boost::shared_ptr<RouterThread> RouterThreadPtr;

} // namespace dmcs


#endif // ROUTER_THREAD_H

// Local Variables:
// mode: C++
// End:
