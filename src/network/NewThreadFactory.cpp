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
 * @file   NewThreadFactory.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Mar  19 10:59:02 2012
 * 
 * @brief  
 * 
 * 
 */

#include "network/NewThreadFactory.h"

namespace dmcs {

NewThreadFactory::NewThreadFactory()
  : request_dispatcher_thread(0),
    output_dispatcher_thread(0),
    joiner_dispatcher_thread(0)
{ }



NewThreadFactory::~NewThreadFactory()
{
  if (request_dispatcher_thread && request_dispatcher_thread->joinable())
    {
      request_dispatcher_thread->interrupt();
      request_dispatcher_thread->join();
      delete request_dispatcher_thread;
      request_dispatcher_thread = 0;
    }

  if (output_dispatcher_thread && output_dispatcher_thread->joinable())
    {
      output_dispatcher_thread->interrupt();
      output_dispatcher_thread->join();
      delete output_dispatcher_thread;
      output_dispatcher_thread = 0;
    }

  if (joiner_dispatcher_thread && joiner_dispatcher_thread->joinable())
    {
      joiner_dispatcher_thread->interrupt();
      joiner_dispatcher_thread->join();
      delete joiner_dispatcher_thread;
      joiner_dispatcher_thread = 0;
    }

  for (BoostThreadVec::iterator it = neighbor_thread_vec.begin();
       it != neighbor_thread_vec.end(); ++it)
    {
      boost::thread* neighbor_thread = *it;
      assert (neighbor_thread);

      if (neighbor_thread->joinable())
	{
	  neighbor_thread->interrupt();
	  neighbor_thread->join();
	  delete neighbor_thread;
	  neighbor_thread = 0;
	}
    }

  for (BoostThreadVec::iterator it = context_thread_vec.begin();
       it != context_thread_vec.end(); ++it)
    {
      boost::thread* context_thread = *it;
      assert(context_thread);

      if (context_thread->joinable())
	{
	  context_thread->interrupt();
	  context_thread->join();
	  delete context_thread;
	  context_thread = 0;
	}
    }
}



void
NewThreadFactory::createMainThreads(NewConcurrentMessageDispatcherPtr md,
				    RequestDispatcherPtr rd,
				    NewOutputDispatcherPtr od,
				    NewJoinerDispatcherPtr jd)
{
  RequestDispatcherWrapper rdw;
  request_dispatcher_thread = new boost::thread(rdw, rd, md);

  NewOutputDispatcherWrapper odw;
  output_dispatcher_thread = new boost::thread(odw, od, md);

  if (jd != NewJoinerDispatcherPtr()) // non-leaf case
    {
      NewJoinerDispatcherWrapper jdw;
      joiner_dispatcher_thread = new boost::thread(jdw, jd, md);
    }
}



void
NewThreadFactory::createNeighborThreads(NewConcurrentMessageDispatcherPtr md,
					NewNeighborVecPtr neighbors)
{
  for (NewNeighborVec::const_iterator it = neighbors->begin();
       it != neighbors->end(); ++it)
    {
      NewNeighborThreadPtr neighbor(new NewNeighborThread(*it));
      NewNeighborThreadWrapper neighbor_wrapper;
      boost::thread* neighbor_thread = new boost::thread(neighbor_wrapper, neighbor, md);
      neighbor_thread_vec.push_back(neighbor_thread);
    }
}



void
NewThreadFactory::createContextThreads(NewContextVecPtr contexts,
				       NewConcurrentMessageDispatcherPtr md,
				       RequestDispatcherPtr rd,
				       NewJoinerDispatcherPtr jd)
{
  for (NewContextVec::const_iterator it = contexts->begin();
       it != contexts->end(); ++it)
    {
      NewContextWrapper ctx_wrapper;
      boost::thread* ctx_thread = new boost::thread(ctx_wrapper, *it, md, rd, jd);
      context_thread_vec.push_back(ctx_thread);
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
