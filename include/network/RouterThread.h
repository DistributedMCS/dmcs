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
#include "network/ConcurrentMessageQueueHelper.h"
#include "mcs/HashedBiMap.h"

#include <boost/thread.hpp>

namespace dmcs {

class RouterThread
{
public:
  RouterThread(ConcurrentMessageQueuePtr& srn,
	       ConcurrentMessageQueueVecPtr& rnn)
    : sat_router_notif(srn), 
      router_neighbors_notif(rnn)
  { }

  void
  operator()()
  {
    while (1)
      {
	// wait for any conflict from the local solver
	ConflictNotification* cn;
	void *ptr         = static_cast<void*>(&cn);
	unsigned int p    = 0;
	std::size_t recvd = 0;

	sat_router_notif->receive(ptr, sizeof(cn), recvd, p);

	if (ptr && cn)
	  {
	    const std::size_t noff = cn->val;
	    
	    DMCS_LOG_TRACE("Got a notification from local solver. noff = " << noff);
	    
	    // inform neighbor at noff about the conflict
	    ConcurrentMessageQueuePtr& cmq = (*router_neighbors_notif)[noff];
	    
	    std::size_t p1 = 0;

	    ConflictNotification* ow_neighbor =
	      (ConflictNotification*) overwrite_send(cmq, &cn, sizeof(cn), p1);

	    if (ow_neighbor)
	      {
		delete ow_neighbor;
		ow_neighbor = 0;
	      }
	  }
	else
	  {
	    DMCS_LOG_FATAL("Got null message: " << ptr << " " << cn);
	    assert (ptr != 0 && cn != 0);
	  }
      }
  }

private:
  ConcurrentMessageQueuePtr     sat_router_notif;
  ConcurrentMessageQueueVecPtr  router_neighbors_notif;
};

typedef boost::shared_ptr<RouterThread> RouterThreadPtr;

} // namespace dmcs


#endif // ROUTER_THREAD_H

// Local Variables:
// mode: C++
// End:
