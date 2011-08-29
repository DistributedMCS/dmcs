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
 * @file   ThreadFactory.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  3 12:39:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef THREAD_FACTORY_H
#define THREAD_FACTORY_H

#include "dmcs/Context.h"
#include "mcs/HashedBiMap.h"
#include "mcs/ProxySignatureByLocal.h"
#include "mcs/ResourceManager.h"
#include "mcs/Theory.h"
#include "network/BaseClient.h"
#include "network/JoinerDispatcher.h"
#include "network/NeighborThread.h"
#include "network/Worker.h"
#include "network/connection.hpp"
#include "solver/RelSatSolver.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

typedef std::vector<boost::thread*> ThreadVec;
typedef boost::shared_ptr<ThreadVec> ThreadVecPtr;

class ResourceManager;

class ThreadFactory
{
public:
  ThreadFactory(const ContextPtr& c, 
		const TheoryPtr& t,
		const SignaturePtr& ls,
		const NeighborListPtr& ns,
		std::size_t sid,
		QueryPlan* qp,
		MessagingGatewayBC* m,
		HashedBiMap* co,
		JoinerDispatcher* jd);

  void
  createNeighborThreads();

  WorkerPtr
  createWorkerThreads(std::size_t path, ResourceManager* rm,
		      std::size_t worker_index);

private:
  const ContextPtr context;
  const TheoryPtr theory;
  const SignaturePtr local_sig;
  const NeighborListPtr nbs;
  ThreadVecPtr neighbor_threads;
  NeighborThreadVecPtr neighbors;
  ConcurrentMessageQueueVecPtr neighbors_notif;
  std::size_t session_id;
  QueryPlan* query_plan;
  MessagingGatewayBC* mg;
  HashedBiMap* c2o;             // hashed bimap from context id to
                                // the offset in the vector of
                                // neighbor message queue
  JoinerDispatcher* joiner_dispatcher;
  std::size_t port;
};

typedef boost::shared_ptr<ThreadFactory> ThreadFactoryPtr;


} // namespace dmcs


#endif // THREAD_FACTORY_H

// Local Variables:
// mode: C++
// End:
