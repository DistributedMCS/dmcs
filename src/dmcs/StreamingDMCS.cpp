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
 * @file   StreamingDMCS.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  20 10:11:24 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "mcs/BeliefState.h"
#include "dmcs/BeliefCombination.h"
#include "dmcs/CommandType.h"
#include "dmcs/Cache.h"
#include "dmcs/Log.h"
#include "dmcs/StreamingDMCS.h"
#include "network/Client.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include <vector>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace dmcs {


StreamingDMCS::StreamingDMCS(const ContextPtr& c, const TheoryPtr& t, 
			     const SignatureVecPtr& s, 
			     const QueryPlanPtr& query_plan_,
			     std::size_t buf_count_)
  : BaseDMCS(c, t, s),
    query_plan(query_plan_),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats)),
    buf_count(buf_count_),
    initialized(false),
    neighbor_threads(new ThreadVec),
    dmcs_sat_notif(new ConcurrentMessageQueue),
    router_neighbors_notif(new ConcurrentMessageQueueVec)
{ }



StreamingDMCS::~StreamingDMCS()
{ }



void
StreamingDMCS::initialize(std::size_t invoker, 
			  std::size_t pack_size, 
			  std::size_t port)
{
  const NeighborListPtr& nbs = ctx->getNeighbors();
  std::size_t no_nbs         = nbs->size(); 

  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Here create mqs and threads. no_nbs = " << no_nbs);

  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
  mg = mqf.createMessagingGateway(port, no_nbs); // we use the port as unique id

  BeliefStatePtr localV;
  if (invoker == 0)
    {
      localV = query_plan->getGlobalV();
    }
  else
    {
      const std::size_t my_id = ctx->getContextID();
      localV = query_plan->getInterface(invoker, my_id);
    }

  const SignaturePtr& local_sig = ctx->getSignature();

  ThreadFactory tf(ctx, theory, local_sig, localV,  pack_size, mg, dmcs_sat_notif);

  sat_thread = tf.createLocalSolveThread();

  if (no_nbs > 0)
    {
      tf.createNeighborThreads(neighbor_threads, router_neighbors_notif);
      join_thread     = tf.createJoinThread(router_neighbors_notif);
      router_thread   = tf.createRouterThread(router_neighbors_notif);
    }
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " All threads created!");
}



void
StreamingDMCS::listen(ConcurrentMessageQueuePtr handler_dmcs_notif,
		      std::size_t& invoker, 
		      std::size_t& pack_size, 
		      std::size_t& port)
{
  // wait for a signal from Handler
  StreamingDMCSNotification* sn;
  void *ptr         = static_cast<void*>(&sn);
  unsigned int p    = 0;
  std::size_t recvd = 0;

  handler_dmcs_notif->receive(ptr, sizeof(sn), recvd, p);

  if (ptr && sn)
    {
      invoker   = sn->invoker;
      pack_size = sn->pack_size;
      port      = sn->port;
    }
  else
    {
      DMCS_LOG_FATAL("Got null message: " << ptr << " " << sn);
      assert(ptr != 0 && sn != 0);
    }

  DMCS_LOG_TRACE("invoker   = " << invoker);
  DMCS_LOG_TRACE("pack_size = " << pack_size);
  DMCS_LOG_TRACE("port      = " << port);
}


void
StreamingDMCS::start_up()
{
  const std::size_t system_size = ctx->getSystemSize();
  const NeighborListPtr& nbs    = ctx->getNeighbors();
  const std::size_t no_nbs      = nbs->size();


  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Trigger SAT solver with empty conflict and empty ass.");

  //BeliefState* empty_model = new BeliefState(system_size, BeliefSet());
  //mg->sendModel(empty_model, 0, ConcurrentMessageQueueFactory::JOIN_OUT_MQ, 0);
  
  Conflict* empty_conflict       = new Conflict(system_size, BeliefSet());
  BeliefState* empty_ass         = new BeliefState(system_size, BeliefSet());
  
  ConflictNotification* mess_sat = new ConflictNotification(0, empty_conflict, empty_ass);
  ConflictNotification* ow_sat = 
    (ConflictNotification*) overwrite_send(dmcs_sat_notif, &mess_sat, sizeof(mess_sat), 0);
  
  if (ow_sat)
    {
      delete ow_sat;
      ow_sat = 0;
    }

  if (no_nbs > 0) // not a leaf context
    {

      DMCS_LOG_TRACE("Intermediate context. Send requests to neighbors by placing a message in each of the NeighborOut's MQ");
      DMCS_LOG_TRACE("router_neighbors_notif.size() = ");
      DMCS_LOG_TRACE(router_neighbors_notif->size());

      assert (router_neighbors_notif->size() == no_nbs);

      Conflict* empty_conflict       = new Conflict(system_size, BeliefSet());
      BeliefState* empty_ass         = new BeliefState(system_size, BeliefSet());
      ConflictNotification* cn = new ConflictNotification(0, empty_conflict, empty_ass);

      for (std::size_t i = 0; i < no_nbs; ++i)
	{
	  DMCS_LOG_TRACE(" First push to offset " << i);
	  ConcurrentMessageQueuePtr& cmq = (*router_neighbors_notif)[i];

	  DMCS_LOG_TRACE(__PRETTY_FUNCTION__ << " Will push: conflict = (" << cn->conflict << ") " << *(cn->conflict)
			 <<", partial_ass = (" << cn->partial_ass << ") " << *(cn->partial_ass));

	  ConflictNotification* ow_neighbor = (ConflictNotification*) overwrite_send(cmq, &cn, sizeof(cn), 0);

	  if (ow_neighbor)
	    {
	      delete ow_neighbor;
	      ow_neighbor = 0;
	    }
	}
    }
}



void
StreamingDMCS::work()
{
  // interrupt SAT thread
}



void
StreamingDMCS::loop(ConcurrentMessageQueuePtr& handler_dmcs_notif)
{
  std::size_t invoker;
  std::size_t pack_size;
  std::size_t port;

  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  while (1)
    {
      listen(handler_dmcs_notif, invoker, pack_size, port);

      if (!initialized)
	{
	  initialize(invoker, pack_size, port);
	  start_up();
	  initialized = true;
	}

      work();
    }
}


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
