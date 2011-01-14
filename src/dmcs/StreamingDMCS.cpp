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
    system_size(c->getSystemSize()),
    my_id(c->getContextID()),
    buf_count(buf_count_),
    initialized(false),
    neighbor_input_threads(new ThreadVec)
{ 
  const NeighborListPtr& nb = ctx->getNeighbors();
  std::cerr << "StreamingDMCS::StreamingDMCS(). nb->size = " << nb->size() << std::endl;
}



StreamingDMCS::~StreamingDMCS()
{ }


void
StreamingDMCS::initialize(std::size_t invoker, std::size_t pack_size, std::size_t port)
{
  const NeighborListPtr& nb = ctx->getNeighbors();
  std::size_t no_nbs = nb->size();

  DMCS_LOG_DEBUG("Here create mqs");
  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
  mg = mqf.createMessagingGateway(port, no_nbs); // we use the port as unique id

  BeliefStatePtr localV;
  if (invoker == 0)
    {
      localV = query_plan->getGlobalV();
    }
  else
    {
      std::size_t my_id = ctx->getContextID();
      localV = query_plan->getInterface(invoker, my_id);
    }

  const SignaturePtr& local_sig = ctx->getSignature();

  ThreadFactory tf(ctx, theory, local_sig, localV,  pack_size, mg);

  sat_thread = tf.createLocalSolveThread();

  if (no_nbs > 0)
    {
      tf.createNeighborInputThreads(neighbor_input_threads);
      BoolNotificationFuturePtr bnf(new BoolNotificationFuture(bnp.get_future()));
      join_thread = tf.createJoinThread(bnf);
    }
}



void
StreamingDMCS::listen(StreamingDMCSNotificationFuturePtr& snf,
		      std::size_t& invoker, 
		      std::size_t& pack_size, 
		      std::size_t& port,
		      Conflict* conflict)
{
  // wait for a signal from Handler
  snf->wait();

  StreamingDMCSNotificationPtr sn = snf->get();

  invoker   = sn->invoker;
  pack_size = sn->pack_size;
  port      = sn->port;
  conflict  = sn->conflict;
}


void
StreamingDMCS::work()
{
  const NeighborListPtr& nb = ctx->getNeighbors();
  std::size_t no_nbs = nb->size();

  if (no_nbs == 0) // this is a leaf context
    {
#ifdef DEBUG
      DMCS_LOG_DEBUG("StreamingDMCS::start_up. Leaf context. Put an empty model into SatInputMQ to start the SAT solver without input");
#endif
      // put an empty model into SatInputMQ to start the SAT solver without input
      std::size_t system_size = ctx->getSystemSize();
      BeliefState* empty_model = new BeliefState(system_size, BeliefSet());
      mg->sendModel(empty_model, 0, ConcurrentMessageQueueFactory::JOIN_OUT_MQ, 0);
    }
  else // this is an intermediate context, now trigger the Joiner
    {
#ifdef DEBUG
      DMCS_LOG_DEBUG("StreamingDMCS::start_up. Intermediate context. Send requests to neighbors by placing a message in each of the NeighborQueryMQ");
#endif

      for (std::size_t i = 0; i < no_nbs; ++i)
	{
	  const std::size_t off = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + 2*i + 1;	  

	  Conflict* empty_conflict = new Conflict(system_size, BeliefSet());
	  mg->sendConflict(empty_conflict, 0, off, 0);
	}
    }
}



void
StreamingDMCS::start_up(StreamingDMCSNotificationFuturePtr& snf)
{
  std::size_t invoker;
  std::size_t pack_size;
  std::size_t port;
  Conflict*   conflict;

  while (1)
    {
      listen(snf, invoker, pack_size, port, conflict);

      if (!initialized)
	{
	  initialize(invoker, pack_size, port);
	  initialized = true;
	}

      work();
    }
}


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
