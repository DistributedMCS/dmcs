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
#include "network/ConflictsAnalysisThread.h"
#include "network/JoinThread.h"
#include "network/NeighborThread.h"
#include "network/OutputThread.h"
#include "network/RelSatSolverThread.h"
#include "network/RouterThread.h"
#include "network/ThreadFactory.h"
#include "solver/SatSolverFactory.h"

namespace dmcs {

ThreadFactory::ThreadFactory(const ContextPtr& context_, 
			     const TheoryPtr& theory_,
			     const SignaturePtr& local_sig_,
			     const BeliefStatePtr& localV_,
			     std::size_t pack_size_,
			     MessagingGatewayBCPtr& mg_,
			     const ConflictNotificationFuturePtr& handler_sat_cnf_)
  : context(context_), theory(theory_), 
    local_sig(local_sig_), localV(localV_),
    pack_size(pack_size_), mg(mg_),
    handler_sat_cnf(handler_sat_cnf_), 
    c2o(new HashedBiMap),
    cnpv(new ConflictNotificationPromiseVec)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  // fill up the map: ctx_id <--> offset
  const NeighborListPtr& nbs = context->getNeighbors();
  std::size_t off = 0;
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++off)
    {
      std::size_t nid = (*it)->neighbor_id;
      c2o->insert(Int2Int(nid, off));
    }
}



void
ThreadFactory::createNeighborThreads(ThreadVecPtr& neighbor_input_threads)
{
  const NeighborListPtr& nbs    = context->getNeighbors();
  const std::size_t ctx_id      = context->getContextID();
  const std::size_t system_size = context->getSystemSize();

  const std::size_t no_nbs      = nbs->size();
  std::size_t i = 0;
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Going to create " << no_nbs << " neighbor threads.");

  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++i)
    {
      const NeighborPtr nb = *it;
      ConflictNotificationPromisePtr cnp(new ConflictNotificationPromise);
      ConflictNotificationFuturePtr  cnf(new ConflictNotificationFuture(cnp->get_future()));

      NeighborThread nt(cnf, mg, nb, c2o, ctx_id, pack_size);
      
      boost::thread* nit = new boost::thread(nt);
      neighbor_input_threads->push_back(nit);

      // notify the neighbor with an empty ConflictNotification, just for starting up
      Conflict* c    = new Conflict(system_size, BeliefSet());
      BeliefState* p = new BeliefState(system_size, BeliefSet());
      ConflictNotificationPtr first_push(new ConflictNotification(c, p));

      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " First push to offset " << i);
      cnp->set_value(first_push);
    }
}



boost::thread*
ThreadFactory::createJoinThread()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  const NeighborListPtr& nbs = context->getNeighbors();
  const std::size_t no_nbs   = nbs->size();

  JoinThread jt(no_nbs, c2o, mg);
  boost::thread* t = new boost::thread(jt);

  return t;
}



boost::thread*
ThreadFactory::createLocalSolveThread()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  const std::size_t my_id       = context->getContextID();
  const std::size_t system_size = context->getSystemSize();
  sat_router_cnp = ConflictNotificationPromisePtr(new ConflictNotificationPromise);

  SatSolverFactory ssf(my_id, theory, local_sig, localV, system_size, mg, handler_sat_cnf, sat_router_cnp);

  RelSatSolverPtr relsatsolver = ssf.create<RelSatSolverPtr>();

  RelSatSolverThread rsst(relsatsolver);
  boost::thread* t = new boost::thread(rsst);

  return t;
}



boost::thread*
ThreadFactory::createRouterThread()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  ConflictNotificationFuturePtr cnf = ConflictNotificationFuturePtr(new ConflictNotificationFuture(sat_router_cnp->get_future()));

  RouterThread rt(cnf, cnpv, c2o);
  boost::thread* t = new boost::thread(rt);

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
