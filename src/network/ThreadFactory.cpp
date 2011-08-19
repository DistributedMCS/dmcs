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
#include "network/JoinThread.h"
#include "network/NeighborThread.h"
#include "network/OutputThread.h"
#include "network/RelSatSolverThread.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {



ThreadFactory::ThreadFactory(const ContextPtr& c, 
			     const TheoryPtr& t,
			     const SignaturePtr& ls,
			     const NeighborListPtr& ns,
			     std::size_t sid,
			     QueryPlan* qp,
			     MessagingGatewayBC* m,
			     HashedBiMap* co,
			     JoinerDispatcher* jd)
  : context(c),
    theory(t), 
    local_sig(ls),
    nbs(ns),
    neighbor_threads(new ThreadVec),
    neighbors(new NeighborThreadVec),
    neighbors_notif(new ConcurrentMessageQueueVec),
    session_id(sid),
    query_plan(qp),
    mg(m),
    c2o(co),
    joiner_dispatcher(jd)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
}



void
ThreadFactory::createNeighborThreads()
{
  const std::size_t ctx_id = context->getContextID();

  DMCS_LOG_TRACE("Going to create " << nbs->size() << " neighbor threads.");

  std::size_t i = 0;
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++i)
    {
      DMCS_LOG_TRACE("Create neighbor thread " << i);

      ConcurrentMessageQueuePtr cmq(new ConcurrentMessageQueue);
      neighbors_notif->push_back(cmq);

      NeighborThread* nt = new NeighborThread();
      neighbors->push_back(nt);

      const NeighborPtr nb = *it;
      
      boost::thread* nit = new boost::thread(*nt, cmq.get(), mg, nb.get(), c2o, ctx_id);
      neighbor_threads->push_back(nit);
    }
}



WorkerPtr
ThreadFactory::createWorkerThreads(std::size_t path)
{
  // the placer holder of threads
  WorkerPtr worker(new Worker);

  ConcurrentMessageQueue* joiner_sat_notif = new ConcurrentMessageQueue;

  // create join thread
  const std::size_t system_size = context->getSystemSize();
  const std::size_t no_nbs = nbs->size();

  // create the joiner and register its input queue to joiner_dispatcher
  JoinThread jt(port, session_id);
  joiner_dispatcher->registerThread(path, jt.getCMQ());
  
  worker->join_thread = new boost::thread(jt, no_nbs, system_size, 
					  mg, joiner_sat_notif, neighbors_notif.get());


  // create solver thread
  bool is_leaf  = (no_nbs == 0);
  const std::size_t my_id = context->getContextID();

  SatSolverFactory ssf(is_leaf, my_id, session_id, theory, local_sig, 
		       c2o, system_size, query_plan, joiner_sat_notif, mg);

  RelSatSolverPtr relsatsolver = ssf.create<RelSatSolverPtr>();

  RelSatSolverThread rsst(relsatsolver);
  worker->sat_thread = new boost::thread(rsst);

  worker->request_mq = rsst.getRequestMQ();

  return worker;
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
