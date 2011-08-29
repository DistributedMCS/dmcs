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
 * @file   RelSatSolverThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  3 15:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/Log.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/RelSatSolverThread.h"

namespace dmcs {

RelSatSolverThread::RelSatSolverThread(const RelSatSolverPtr& rss,
				       ResourceManager* r,
				       std::size_t wkid)
  : relsatsolver(rss),
    request_mq(new ConcurrentMessageQueue),
    rm(r),
    worker_index(wkid)
{ }



RelSatSolverThread::~RelSatSolverThread()
{
  if (request_mq)
    {
      delete request_mq;
      request_mq = 0;
    }
}


void
RelSatSolverThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  while (1)
    {
      try 
	{
	  // wait for request from request_mq
	  std::size_t prio = 0;
	  int timeout = 0;
	  StreamingForwardMessage* sfMess = receive_incoming_message(request_mq);

	  assert (sfMess);

	  std::size_t invoker = sfMess->getInvoker();
	  std::size_t path = sfMess->getPath();
	  std::size_t session_id = sfMess->getSessionId();
	  std::size_t k1 = sfMess->getK1();
	  std::size_t k2 = sfMess->getK2();

	  delete sfMess;
	  sfMess = 0;

	  relsatsolver->solve(invoker, path, session_id, k1, k2);

	  // inform ResourceManager about my status
	  rm->updateStatus(worker_index, request_mq, false, k1, k2);	  
	}
      catch(const boost::thread_interrupted& ex)
	{
	  DMCS_LOG_TRACE("Got interrupted, will now restart.");
	  
	  // reset sat solver
	  relsatsolver->refresh(); // including remove_input
	}
    }
}



ConcurrentMessageQueue*
RelSatSolverThread::getRequestMQ()
{
  return request_mq;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
