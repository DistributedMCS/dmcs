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


#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include "network/Client.h"

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
    neighbor_threads(new ThreadVec)
{ 
  const NeighborListPtr& nb = ctx->getNeighbors();
  std::cerr << "StreamingDMCS::StreamingDMCS(). nb->size = " << nb->size() << std::endl;
}



StreamingDMCS::~StreamingDMCS()
{ }


void
StreamingDMCS::initialize(std::size_t invoker, 
			  std::size_t pack_size, 
			  std::size_t port,
			  ConflictNotificationFuturePtr& cnf)
{
  DMCS_LOG_DEBUG("Here create mqs");
  const NeighborListPtr& nbs = ctx->getNeighbors();
  std::size_t no_nbs         = nbs->size(); 

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

  ThreadFactory tf(ctx, theory, local_sig, localV,  pack_size, mg, cnf);

  sat_thread = tf.createLocalSolveThread();

  if (no_nbs > 0)
    {
      
      ConflictNotificationPromiseVecPtr cnpv(new ConflictNotificationPromiseVec);
      tf.createNeighborThreads(neighbor_threads, cnpv);
      join_thread     = tf.createJoinThread();
      router_thread   = tf.createRouterThread(cnpv);
    }
}



void
StreamingDMCS::listen(StreamingDMCSNotificationFuturePtr& snf,
		      std::size_t& invoker, 
		      std::size_t& pack_size, 
		      std::size_t& port)
{
  // wait for a signal from Handler
  snf->wait();

  StreamingDMCSNotificationPtr sn = snf->get();

  invoker   = sn->invoker;
  pack_size = sn->pack_size;
  port      = sn->port;
}


void
StreamingDMCS::start_up()
{
  const std::size_t system_size = ctx->getSystemSize();
  const NeighborListPtr& nbs    = ctx->getNeighbors();
  const std::size_t no_nbs      = nbs->size();

  if (no_nbs == 0) // this is a leaf context
    {
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Leaf context. Put an empty model into JOIN_OUT_MQ to start the SAT solver without input");


      BeliefState* empty_model = new BeliefState(system_size, BeliefSet());
      mg->sendModel(empty_model, 0, ConcurrentMessageQueueFactory::JOIN_OUT_MQ, 0);
    }
  else
    {

      /*
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Intermediate context. Send requests to neighbors by placing a message in each of the NEIGHBOR_OUT_MQ");

      for (std::size_t i = 0; i < no_nbs; ++i)
	{
	  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Send empty conflict to offset 5 + " << i);

	  const std::size_t off = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + i;

	  Conflict* empty_conflict = new Conflict(system_size, BeliefSet());
	  mg->sendConflict(empty_conflict, 0, off, 0);
	}
      */
    }
}



void
StreamingDMCS::work()
{
  // interrupt SAT thread
}



void
StreamingDMCS::loop(StreamingDMCSNotificationFuturePtr& snf, ConflictNotificationFuturePtr& cnf)
{
  std::size_t invoker;
  std::size_t pack_size;
  std::size_t port;

  while (1)
    {
      listen(snf, invoker, pack_size, port);

      if (!initialized)
	{
	  initialize(invoker, pack_size, port, cnf);
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
