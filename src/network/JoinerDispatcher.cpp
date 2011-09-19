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
 * @file   JoinerDispatcher.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Aug  16 10:23:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/Log.h"
#include "network/JoinerDispatcher.h"

namespace dmcs {

JoinerDispatcher::JoinerDispatcher()
  : BaseDispatcher()
{ }



void
JoinerDispatcher::operator()(MessagingGatewayBC* mg)
{
  std::size_t prio = 0;
  int timeout = 0;

  while (1)
    {
      struct MessagingGatewayBC::JoinIn ji =
	mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio, timeout);

      DMCS_LOG_TRACE("Got JoinIn = {" << ji.ctx_offset << ", " << ji.peq_cnt << "}");

      // offset of the neighbor in the vector of Joiner
      std::size_t noff = ji.ctx_offset; 

      // offset of the message queue storing models from the neighbor
      const std::size_t offset = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + noff;

      // now read ji.peq_cnt models and also send each to the corresponding join thread
      ConcurrentMessageQueue* cmq;
      std::size_t path = 0;
      for (std::size_t i = 0; i < ji.peq_cnt; ++i)
	{
	  struct MessagingGatewayBC::ModelSession ms = mg->recvModel(offset, prio, timeout);
	  path = ms.path;
	  // find the corresponding join thread
	  DMCS_LOG_TRACE("Finding path = " << path);
	  CMQMap::const_iterator it = thread_map->find(path);

	  assert (it != thread_map->end());

	  struct MessagingGatewayBC::ModelSessionOffset 
	    mso = { ms.m, ms.path, ms.sid, noff };

	  cmq = it->second;

	  if (ms.m)
	    {
	      DMCS_LOG_TRACE("Send " << *ms.m << " from noff = " << noff << " to " << cmq << ". path = " << path);
	    }
	  else
	    {
	      DMCS_LOG_TRACE("Send NULL from noff = " << noff << " to " << cmq << ". path = " << path);
	    }

	  cmq->send(&mso, sizeof(mso), 0);
	}
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
