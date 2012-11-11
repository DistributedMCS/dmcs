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
 * @file   CycleBreaker.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  11 18:30:24 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/CycleBreaker.h"

namespace dmcs {

void
CycleBreaker::startup(NewConcurrentMessageDispatcherPtr md,
		      RequestDispatcherPtr rd,
		      NewJoinerDispatcherPtr jd)
{
  int timeout = 0;
  while (1)
    {
      DBGLOG(DBG, "CycleBreaker[" << ctx_id << "]::startup(): Waiting at CYCLE_BREAKER_MQ[" << ctx_offset << "]");
      // Listen to the CYCLE_BREAKER_MQ
      ForwardMessage* fwd_mess = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::CYCLE_BREAKER_MQ, ctx_offset, timeout);
      
      DBGLOG(DBG, "CycleBreaker[" << ctx_id << "]::startup(): Got message: " << *fwd_mess);
      
      std::size_t parent_qid = fwd_mess->qid;
      if (is_shutdown(parent_qid))
	{
	  break;
	}
      
      std::size_t k1 = fwd_mess->k1;
      std::size_t k2 = fwd_mess->k2;

      // Bad requests are not allowed
      assert ((k1 == 0 && k2 == 0) || (0 < k1 && k1 < k2+1));


    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
