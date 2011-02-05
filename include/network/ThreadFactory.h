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
#include "mcs/Theory.h"
#include "network/BaseClient.h"
#include "network/NeighborThread.h"
#include "network/connection.hpp"
#include "solver/RelSatSolver.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

typedef std::vector<boost::thread*> ThreadVec;
typedef boost::shared_ptr<ThreadVec> ThreadVecPtr;

class ThreadFactory
{
public:
  ThreadFactory(bool cd,
		const ContextPtr& c, 
		const TheoryPtr& t,
		const SignaturePtr& ls,
		const BeliefStatePtr& lV,
		const VecSizeTPtr& oss,
		const NeighborListPtr& ns,
		std::size_t ps,
		std::size_t sid,
		MessagingGatewayBC* m,
		ConcurrentMessageQueue* dsn,
		ConcurrentMessageQueue* srn,
		ConcurrentMessageQueue* sjn,
		HashedBiMap* co,
		std::size_t p);

  void
  createNeighborThreads(ThreadVecPtr& neighbor_threads,
			NeighborThreadVecPtr& neighbors,
			ConcurrentMessageQueueVecPtr& router_neighbors_notif);

  boost::thread*
  createJoinThread(ConcurrentMessageQueueVecPtr& joiner_neighbors_notif,
		   ConcurrentMessageQueuePtr& sat_joiner_notif,
		   ConflictVec* conflicts,
		   PartialBeliefState* partial_ass,
		   Decisionlevel* decision);

  boost::thread*
  createLocalSolveThread(boost::thread* join_thread);

  boost::thread*
  createRouterThread(ConcurrentMessageQueueVecPtr& router_neighbors_notif);

  //boost::thread*
  //createOutputThread(const connection_ptr& conn_);

private:
  bool conflicts_driven;
  const ContextPtr                context;
  const TheoryPtr                 theory;
  const SignaturePtr              local_sig;
  const BeliefStatePtr            localV;
  const VecSizeTPtr               orig_sigs_size;
  const NeighborListPtr           nbs;
  std::size_t                     pack_size;
  std::size_t                     session_id;
  MessagingGatewayBC*             mg;
  ConcurrentMessageQueue*         dmcs_sat_notif;
  ConcurrentMessageQueue*         sat_router_notif;			
  ConcurrentMessageQueue*         sat_joiner_notif;
  HashedBiMap*                    c2o;             // hashed bimap from context id to
			                           // the offset in the vector of
				                   // neighbor message queue
  std::size_t port;
};


} // namespace dmcs


#endif // THREAD_FACTORY_H

// Local Variables:
// mode: C++
// End:
