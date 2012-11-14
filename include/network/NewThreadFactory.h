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
 * @file   NewThreadFactory.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Mar  19 10:45:50 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_THREAD_FACTORY_H
#define NEW_THREAD_FACTORY_H

#include <boost/thread.hpp>

#include "network/NewJoinerDispatcher.h"
#include "network/NewOutputDispatcher.h"
#include "mcs/NewNeighbor.h"
#include "mcs/Registry.h"
#include "network/RequestDispatcher.h"
#include "network/NewNeighborThread.h"

namespace dmcs {

typedef std::vector<boost::thread*> BoostThreadVec;

class NewThreadFactory
{
public:
  NewThreadFactory();

  ~NewThreadFactory();

  void
  createMainThreads(NewConcurrentMessageDispatcherPtr md,
		    RequestDispatcherPtr rd,
		    NewOutputDispatcherPtr od,
		    NewJoinerDispatcherPtr jd = NewJoinerDispatcherPtr());

  void
  createNeighborThreads(NewConcurrentMessageDispatcherPtr md,
			NewNeighborVecPtr neighbors);

  void
  createContextThreads(NormalContextVecPtr contexts,
		       NewConcurrentMessageDispatcherPtr md,
		       RequestDispatcherPtr rd,
		       NewJoinerDispatcherPtr jd);

private:
  boost::thread* request_dispatcher_thread;
  boost::thread* output_dispatcher_thread;
  boost::thread* joiner_dispatcher_thread;
  
  BoostThreadVec context_thread_vec;
  BoostThreadVec neighbor_thread_vec;
};


typedef boost::shared_ptr<NewThreadFactory> NewThreadFactoryPtr;

} // namespace dmcs

#endif // NEW_THREAD_FACTORY_H

// Local Variables:
// mode: C++
// End:
