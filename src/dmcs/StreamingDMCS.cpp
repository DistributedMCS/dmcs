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
#include "dmcs/ConflictNotification.h"
#include "dmcs/Log.h"
#include "dmcs/StreamingDMCS.h"
#include "network/Client.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include <numeric>
#include <vector>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace dmcs {
  
StreamingDMCS::StreamingDMCS(const ContextPtr& c, 
			     const TheoryPtr& t, 
			     const SignatureVecPtr& s, 
			     const QueryPlanPtr& qp)
  : BaseDMCS(c, t, s),
    query_plan(qp),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats)),
    initialized(false),
    sat_thread(0),
    join_thread(0),
    neighbor_threads(new ThreadVec),
    neighbors(new NeighborThreadVec),
    dmcs_sat_notif(new ConcurrentMessageQueue),
    neighbors_notif(new ConcurrentMessageQueueVec),
    c2o(new HashedBiMap),
    first_round(true),
    my_starting_session_id(0)
  { }



StreamingDMCS::~StreamingDMCS()
{
  DMCS_LOG_TRACE("Join SAT thread");

  if (sat_thread && sat_thread->joinable())
    {
      sat_thread->interrupt();
      sat_thread->join();
    }
  else
    {
      DMCS_LOG_ERROR("SAT thread is not joinable");
    }

  DMCS_LOG_TRACE("Join JOIN thread");
  
  if (join_thread && join_thread->joinable())
    {
      join_thread->interrupt();
      join_thread->join();
    }
  else
    {
      DMCS_LOG_ERROR("JOIN thread is not joinable");
    }

  ConcurrentMessageQueueVec::iterator rbeg = neighbors_notif->begin();
  ConcurrentMessageQueueVec::iterator rend = neighbors_notif->end();

  DMCS_LOG_TRACE("Informing " << std::distance(rbeg, rend) << " neighbors.");

  // inform all neighbors about the shutdown
  for (ConcurrentMessageQueueVec::iterator it = rbeg; it != rend; ++it)
    {
      ConflictNotification* neighbor_mess = new ConflictNotification(0, ConflictNotification::FROM_DMCS, ConflictNotification::SHUTDOWN);
      ConflictNotification* ow_neighbor =
	(ConflictNotification*) overwrite_send(it->get(), &neighbor_mess, sizeof(neighbor_mess), 0);

      DMCS_LOG_TRACE("Sent SHUTDOWN to neighbor " << std::distance(it, rend));

      if (ow_neighbor)
	{
	  delete ow_neighbor;
	  ow_neighbor = 0;
	}
    }


  ThreadVec::iterator beg = neighbor_threads->begin();
  ThreadVec::iterator end = neighbor_threads->end();

  NeighborThreadVec::iterator nbeg = neighbors->begin();
  NeighborThreadVec::iterator nend = neighbors->end();

  assert(std::distance(beg,end) == std::distance(nbeg,nend));

  DMCS_LOG_TRACE("Stop " <<  std::distance(beg, end) << " NEIGHBOR threads");

  for (NeighborThreadVec::iterator nit = nbeg; nit != nend; ++nit)
    {
      DMCS_LOG_TRACE("Stop NEIGHBOR thread #" << std::distance(nit, nend));

      assert(*nit);

      (*nit)->stop();
    }

  DMCS_LOG_TRACE("Join " <<  std::distance(beg, end) << " NEIGHBOR threads");

  for (ThreadVec::iterator it = beg; it != end; ++it)
    {
      DMCS_LOG_TRACE("Join NEIGHBOR thread #" << std::distance(it, end));

      assert(*it);

      (*it)->interrupt();
      (*it)->join();
    }

  DMCS_LOG_TRACE("Cleanup threads");

  if (sat_thread) { delete sat_thread; sat_thread = 0; }
  if (join_thread) { delete join_thread; join_thread = 0; }

  DMCS_LOG_TRACE("Good night, and good luck.");
}



void
StreamingDMCS::initialize(std::size_t parent_session_id,
			  std::size_t invoker, 
			  std::size_t pack_size, 
			  std::size_t port)
{
  std::size_t my_id = ctx->getContextID();
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);
  std::size_t no_nbs         = nbs->size(); 

  DMCS_LOG_TRACE(port << ": Here create mqs and threads. no_nbs = " << no_nbs);

  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
  if (pack_size == 0)
    {
      mg = mqf.createMessagingGateway(port, no_nbs, DEFAULT_PACK_SIZE); // we use the port as unique id
    }
  else
    {
      mg = mqf.createMessagingGateway(port, no_nbs, pack_size);
    }

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

  ThreadFactory tf(ctx, theory, local_sig, localV,  
		   nbs, pack_size, my_starting_session_id,
		   mg.get(), dmcs_sat_notif.get(),
		   c2o.get(), port);



  if (no_nbs > 0)
    {
      tf.createNeighborThreads(neighbor_threads, neighbors, neighbors_notif);
      join_thread = tf.createJoinThread(neighbors_notif, pack_size);
    }

  sat_thread = tf.createLocalSolveThread();

  DMCS_LOG_TRACE(port << ": All threads created!");
}



void
StreamingDMCS::listen(ConcurrentMessageQueue* handler_dmcs_notif,
		      std::size_t& parent_session_id,
		      std::size_t& invoker, 
		      std::size_t& pack_size, 
		      std::size_t& port,
		      StreamingDMCSNotification::NotificationType& type)
{
  // wait for a signal from Handler
  StreamingDMCSNotification* sn = 0;
  void *ptr         = static_cast<void*>(&sn);
  unsigned int p    = 0;
  std::size_t recvd = 0;

  handler_dmcs_notif->receive(ptr, sizeof(sn), recvd, p);

  if (ptr && sn)
    {
      parent_session_id = sn->session_id;
      invoker     = sn->invoker;
      pack_size   = sn->pack_size;
      port        = sn->port;
      type        = sn->type;

      // safe to delete sn here, pointers inside it are preserved
      //delete sn;
      //sn = 0;
    }
  else
    {
      DMCS_LOG_FATAL(port << ": Got null message: " << ptr << " " << sn);
      assert(ptr != 0 && sn != 0);
    }

  DMCS_LOG_TRACE(port << ": invoker = " << invoker << ", pack_size = " << pack_size << ", port = " << port << ", type = " << type);
}


void
StreamingDMCS::start_up(std::size_t parent_session_id,
			std::size_t port)
{
  std::size_t my_id = ctx->getContextID();
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);
  const std::size_t no_nbs = nbs->size();

  if (no_nbs > 0) 
    {
      DMCS_LOG_TRACE(port << ": Intermediate context. Send requests to neighbors by placing a message in each of the NeighborOut's MQ");
      
      DMCS_LOG_TRACE(port << ": neighbors_notif.size() = " << neighbors_notif->size());
      
      assert (neighbors_notif->size() == no_nbs);
      
      for (std::size_t i = 0; i < no_nbs; ++i)
	{
	  DMCS_LOG_TRACE(port << ": First push to offset " << i);
	  ConcurrentMessageQueuePtr& cmq = (*neighbors_notif)[i];
	  
	  ConflictNotification* cn = new ConflictNotification(my_starting_session_id, ConflictNotification::FROM_DMCS, ConflictNotification::REQUEST);
	  
	  ConflictNotification* ow_neighbor = (ConflictNotification*) overwrite_send(cmq.get(), &cn, sizeof(cn), 0);
	  
	  if (ow_neighbor)
	    {
	      delete ow_neighbor;
	      ow_neighbor = 0;
	    }
	}
    }

  DMCS_LOG_TRACE(port << ": Wake up SAT solver.");
  ConflictNotification* mess_sat = new ConflictNotification(parent_session_id, ConflictNotification::FROM_DMCS, ConflictNotification::REQUEST);
  ConflictNotification* ow_sat = 
    (ConflictNotification*) overwrite_send(dmcs_sat_notif.get(), &mess_sat, sizeof(mess_sat), 0);
  
  if (ow_sat)
    {
      delete ow_sat;
      ow_sat = 0;
    }
}



void
StreamingDMCS::loop(ConcurrentMessageQueue* handler_dmcs_notif)
{
  std::size_t parent_session_id = 0;
  std::size_t invoker = 0;
  std::size_t pack_size = 0;
  std::size_t port = 0;

  StreamingDMCSNotification::NotificationType type = StreamingDMCSNotification::REQUEST;

  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  while (1)
    {
      listen(handler_dmcs_notif, 
	     parent_session_id,
	     invoker, pack_size, 
	     port, type);

      if (type == StreamingDMCSNotification::SHUTDOWN)
	{
	  break;
	}

      if (!initialized)
	{
	  initialize(parent_session_id, invoker, pack_size, port);
	  initialized = true;
	}
      start_up(parent_session_id, port);
    }
}


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
