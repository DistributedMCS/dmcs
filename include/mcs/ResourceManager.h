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
 * @file   ResourceManager.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Aug  18 15:41:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include "network/ConcurrentMessageQueueFactory.h"
#include "network/JoinerDispatcher.h"
#include "network/JoinThread.h"
#include "network/RelSatSolverThread.h"
#include "network/ThreadFactory.h"
#include "solver/RelSatSolver.h"

#include <boost/thread/mutex.hpp>

namespace dmcs {

class ResourceManager
{
public:
  ResourceManager(std::size_t mr,
		  ThreadFactory* tf);

  // Handler can request a worker via this method.
  // Depends on the availability of workers (non-busy) 
  // and whether we have reached max_resource, we either 
  // pick an existing, non-busy worker or create a new one 
  // to return, or wait (then Handler will be blocked and 
  // the block will be pushed back automatically)
  ConcurrentMessageQueue*
  requestWorker(std::size_t path);

  // let SatSolver update its status via this method
  void
  updateStatus(std::size_t index, 
	       ConcurrentMessageQueue* sat_cmq,
	       bool bs, std::size_t k_one, std::size_t k_two);

private:
  // + create a pair of SatSolver-Joiner,
  // + give SatSolver an index in the vector of status (workers)
  // + connect SatSolver and Joiner
  // + start RelSatSolverThread
  // + and return the message queue of SatSolver so that any one who receives this 
  // can trigger the solver by sending a request to the message queue
  ConcurrentMessageQueue*
  createWorker(std::size_t path);

private:
  std::size_t max_resource;
  ThreadFactory* thread_factory;
  WorkerVecPtr workers;
  boost::mutex mtx;
};

} // namespace dmcs


#endif // RESOURCE_MANAGER_H

// Local Variables:
// mode: C++
// End:
