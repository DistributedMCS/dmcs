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

#include "dmcs/Log.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/JoinThread.h"
#include "network/NeighborThread.h"
#include "network/OutputThread.h"
#include "network/RelSatSolverThread.h"
#include "network/RouterThread.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {

ThreadFactory::ThreadFactory(bool cd,
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
			     std::size_t p)
  : conflicts_driven(cd),
    context(c),
    theory(t), 
    local_sig(ls),
    localV(lV),
    orig_sigs_size(oss),
    nbs(ns),
    pack_size(ps),
    session_id(sid),
    mg(m),
    dmcs_sat_notif(dsn), 
    sat_router_notif(srn), 
    sat_joiner_notif(sjn),
    c2o(co),
    port(p)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  // fill up the map: ctx_id <--> offset
  std::size_t off = 0;
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++off)
    {
      std::size_t nid = (*it)->neighbor_id;
      c2o->insert(Int2Int(nid, off));
    }
}



void
ThreadFactory::createNeighborThreads(ThreadVecPtr& neighbor_input_threads,
				     NeighborThreadVecPtr& neighbors,
				     ConcurrentMessageQueueVecPtr& router_neighbors_notif)
{
  const std::size_t ctx_id      = context->getContextID();

  DMCS_LOG_TRACE("Going to create " << nbs->size() << " neighbor threads.");

  std::size_t i = 0;
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++i)
    {
      DMCS_LOG_TRACE("Create neighbor thread " << i);

      ConcurrentMessageQueuePtr cmq(new ConcurrentMessageQueue);
      router_neighbors_notif->push_back(cmq);

      NeighborThread* nt = new NeighborThread(port);
      neighbors->push_back(nt);

      const NeighborPtr nb = *it;
      
      boost::thread* nit = new boost::thread(*nt, cmq.get(), mg, nb.get(), c2o, ctx_id, pack_size);
      neighbor_input_threads->push_back(nit);

      DMCS_LOG_TRACE("Created neighbor thread " << i << " for " << port);
    }
}



boost::thread*
ThreadFactory::createJoinThread(ConcurrentMessageQueueVecPtr& joiner_neighbors_notif,
				ConcurrentMessageQueuePtr& sat_joiner_notif,
				ConflictVec* conflicts,
				PartialBeliefState* partial_ass,
				Decisionlevel* decision)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  const std::size_t system_size = context->getSystemSize();
  const std::size_t no_nbs   = nbs->size();

  JoinThread jt(port, session_id);
  boost::thread* t = new boost::thread(jt, no_nbs, system_size, mg, joiner_neighbors_notif.get(),
				       sat_joiner_notif.get(), conflicts, partial_ass, decision);

  return t;
}



boost::thread*
ThreadFactory::createLocalSolveThread(boost::thread* join_thread)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  bool is_leaf                  = (nbs->size() == 0);
  const std::size_t my_id       = context->getContextID();
  const std::size_t system_size = context->getSystemSize();

  SatSolverFactory ssf(is_leaf, conflicts_driven, my_id, session_id, theory, local_sig, 
		       localV, orig_sigs_size, c2o, system_size, mg, dmcs_sat_notif, 
		       sat_router_notif, sat_joiner_notif, join_thread, port);

  RelSatSolverPtr relsatsolver = ssf.create<RelSatSolverPtr>();

  RelSatSolverThread rsst(relsatsolver);
  boost::thread* t = new boost::thread(rsst);

  return t;
}



boost::thread*
ThreadFactory::createRouterThread(ConcurrentMessageQueueVecPtr& router_neighbors_notif)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  RouterThread rt(port);
  boost::thread* t = new boost::thread(rt, sat_router_notif, router_neighbors_notif.get());

  return t;
}


  /*
boost::thread*
ThreadFactory::createOutputThread(const connection_ptr& conn)
{
  OutputThread ots(conn, pack_size, mg, onf);
  boost::thread* t = new boost::thread(ots);

  return t;
  }*/

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
