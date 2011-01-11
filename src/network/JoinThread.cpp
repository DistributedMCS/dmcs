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
 * @file   ThreadFactory.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "network/JoinThread.h"

#include "dmcs/Log.h"

namespace dmcs {


JoinThread::JoinThread(std::size_t expecting_,
		       const HashedBiMapPtr& c2o_,
		       boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : expecting(expecting_),
    c2o(c2o_),
    mg(mg_)
{ }


void
JoinThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  bool stop = false;
  BeliefStatePackagePtr partial_eqs(new BeliefStatePackage(expecting));

  while (!stop)
    {
#ifdef DEBUG
      std::cerr << "JoinThread::operator()(). expecting = " << expecting << std::endl;
#endif

      // look at JOIN_IN_MQ for notification of new models arrival
      std::size_t prio = 0;
      MessagingGateway<BeliefState, Conflict>::JoinIn neighbor_notification = mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio);
      std::size_t ctx_id = neighbor_notification.ctx_id;
      std::size_t peq_cnt = neighbor_notification.peq_cnt;

      // read BeliefState* from NEIGHBOR_MQ
      
      
      expecting--;
      if (expecting == 0)
	{
	  // time to join
	}

      ///@todo: determine the condition for stop = true;
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
