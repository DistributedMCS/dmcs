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
 * @file   OutputDispatcher.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jul  29 10:23:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/Log.h"
#include "network/OutputDispatcher.h"

namespace dmcs {

OutputDispatcher::OutputDispatcher()
  : output_thread_map(new CMQMap)
{ } 



void
OutputDispatcher::registerOutputThread(std::size_t path, ConcurrentMessageQueue* cmq)
{
  // make sure that this output thread has never been register
  CMQMap::const_iterator it = output_thread_map->find(path);
  assert (it == output_thread_map->end());
  
  std::pair<std::size_t, ConcurrentMessageQueue*> ot(path, cmq);
  output_thread_map->insert(ot);
}



void
OutputDispatcher::operator()(MessagingGatewayBC* mg)
{
  std::size_t prio = 0;
  int timeout = 0;

  while (1)
    {
      struct MessagingGatewayBC::ModelSession ms =
	mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

      PartialBeliefState* bs = ms.m;
      std::size_t path = ms.path;
      std::size_t sid = ms.sid;
      if (bs == 0)
	{
	  DMCS_LOG_TRACE("Received: NULL");
	}
      else
	{
	  DMCS_LOG_TRACE("Received: " << *bs << " " << path << " " << sid);
	}

      // find the corresponding output thread
      CMQMap::const_iterator it = output_thread_map->find(path);

      // which must be in the registration list
      assert (it != output_thread_map->end());

      ConcurrentMessageQueue* cmq = it->second;

      DMCS_LOG_TRACE("Send to the right output thread");
      // now send the model to the right output thread (dispatching step)
      cmq->send(&ms, sizeof(ms), 0);
    }
}

}


// Local Variables:
// mode: C++
// End:
