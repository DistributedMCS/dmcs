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
 * @file   PrimitiveDMCS.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Nov  18 18:03:24 2009
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "BeliefCombination.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ClaspProcess.h"
#include "Client.h"
#include "CommandType.h"
#include "DimacsVisitor.h"
#include "ParserDirector.h"
#include "PrimitiveDMCS.h"
#include "Cache.h"

#include "QueryPlan.h"
#include "BeliefState.h"

#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace dmcs {


PrimitiveDMCS::PrimitiveDMCS(const ContextPtr& c, const TheoryPtr& t)
  : BaseDMCS(c,t),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats))
{ }



PrimitiveDMCS::~PrimitiveDMCS()
{ }



PrimitiveDMCS::dmcs_return_type
PrimitiveDMCS::getBeliefStates(PrimitiveMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id

  assert(n > 0 && k <= n);


#ifdef DMCS_STATS_INFO
  // initialize the statistic information
  StatsInfosPtr sis(new StatsInfos);
  
  for (std::size_t i = 0; i < n; ++i)
    {
      TimeDurationPtr t_i(new TimeDuration(0, 0, 0, 0));
      TimeDurationPtr c_i(new TimeDuration(0, 0, 0, 0));
      TimeDurationPtr p_i(new TimeDuration(0, 0, 0, 0));
      TransferTimesPtr tf_i(new TransferTimes);
      
      StatsInfoPtr si(new StatsInfo(t_i, c_i, p_i, tf_i));
      
      sis->push_back(si);
    }
  
  TimeDurationPtr time_combine(new TimeDuration(0, 0, 0, 0));

  std::cerr << "Initialization of the statistic information: " << *sis << std::endl;

  TransferTimesPtr time_transfer(new TransferTimes);
#endif


  const QueryPlanPtr query_plan = ctx->getQueryPlan();

  const BeliefStatePtr& V = mess.getV();

#if defined(DEBUG)
  std::cerr << "In PrimitiveDMCS, at context " << k << std::endl;
#endif // DEBUG

#if 0
  BeliefStateListPtr bs = cache->cacheHit(V);

  if (bs.belief_states_ptr) 
    {
#if defined(DEBUG)
      std::cerr << "cache hit" << std::endl;
#endif //DEBUG
      belief_states = bs;
      return belief_states;
    }
#endif

  // No cache found, we need to compute from scratch
  // Compute all of our local belief states


  // This will give us local_belief_states

  BeliefStateListPtr local_belief_states;

  STATS_DIFF (local_belief_states = localSolve(V),
	      time_lsolve);

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
  printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif // DEBUG

  BeliefStateListPtr belief_states(new BeliefStateList);

  STATS_DIFF (project_to(local_belief_states, V, belief_states),
	      time_projection);


#if defined(DEBUG)
  std::cerr << "Projected belief states..." << std::endl;
  std::cerr << belief_states << std::endl;
  std::cerr << "The V used in projection..." << std::endl;
  std::cerr << V << std::endl;
  printBeliefStatesNicely(std::cerr, belief_states, V, query_plan);
  std::cerr << "Now check for neighbors..." << std::endl;
#endif // DEBUG


  //
  // detect cycles
  //

  const History& hist = mess.getHistory();
  const NeighborsPtr& nbs = query_plan->getNeighbors(k);


  if ((hist.find(k) != hist.end()) || nbs->empty())
    {
#if defined(DEBUG)
      if (nbs->empty())
	{
	  std::cerr << "Reached a leaf context " << k << std::endl;

	}
      else
	{
	  std::cerr << "Cycle detected at context " << k << std::endl;
	}
#endif // DEBUG

      //
      // Do nothing else, local belief states will be sent back to the invoker
      //
    }
  else 
    {
      // We are now at an intermediate context.
      // Need to consult all neighbors before combining with our local belief states

      for (Neighbors::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
	{
	  boost::asio::io_service io_service;
	  boost::asio::ip::tcp::resolver resolver(io_service);

	  std::size_t neighbor_id = *it;

	  boost::asio::ip::tcp::resolver::query query(ctx->getQueryPlan()->getHostname(neighbor_id),
						      ctx->getQueryPlan()->getPort(*it));
	  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
	  boost::asio::ip::tcp::endpoint endpoint = *res_it;
	  
	  mess.insertHistory(k);

#if defined(DEBUG)
	  std::cerr << "Invoking neighbor " << neighbor_id << std::endl;
#endif // DEBUG

	  Client<PrimitiveCommandType> client(io_service, res_it, mess);
	  
	  io_service.run();

	  dmcs_return_type client_mess = client.getResult();


#ifdef DMCS_STATS_INFO
	  BeliefStateListPtr neighbor_belief_states = client_mess->getBeliefStates();
	  StatsInfosPtr stats_infos = client_mess->getStatsInfo();

	  combine(sis, stats_infos);

	  PTimePtr sent_moment = client_mess->getSendingMoment();
	  PTime this_moment = boost::posix_time::microsec_clock::local_time();
	  TimeDurationPtr time_neighbor_me(new TimeDuration(0, 0, 0, 0));
	  *time_neighbor_me = this_moment - (*sent_moment);

	  time_transfer->insert(std::pair<std::size_t, TimeDurationPtr>(neighbor_id, time_neighbor_me));
#else
	  BeliefStateListPtr neighbor_belief_states = client_mess;
#endif // DMCS_STATS_INFO


	  mess.removeHistory(k);


#if defined(DEBUG)
	  std::cerr << "Belief states received from neighbor " << neighbor_id << std::endl;	  
	  std::cerr << neighbor_belief_states << std::endl;
	  std::cerr << "Going to combine " << "k = " << k << " neighbor = " << neighbor_id << std::endl;
#endif // DEBUG


	  STATS_DIFF_REUSE (belief_states = combine(belief_states,
						    neighbor_belief_states,
						    V),
			    time_combine
			    );


#if defined(DEBUG)
	  std::cerr << "Accumulated combination... " << std::endl;	  	  
	  std::cerr << belief_states << std::endl;
#endif // DEBUG
	}
      
#if 0
      cache->insert(V, belief_states);
#endif
    }

#ifdef DEBUG
  if (!hist.empty())
    {
      History::const_iterator ed = hist.end();
      --ed;
      std::size_t invoker = *ed;
      std::cerr << "Going to send " << belief_states->size()
		<< " belief states above back to invoker: C_" << invoker << std::endl;
    }
  else
    {
      std::cerr << "Going to send " << belief_states->size()
		<< " belief states above back to user." << std::endl;
    }
 
  printBeliefStatesNicely(std::cerr, belief_states, V, query_plan);
#endif // DEBUG
  
#ifdef DMCS_STATS_INFO
  StatsInfoPtr my_stats_info = (*sis)[k-1];

  TimeDurationPtr  my_lsolve     = my_stats_info->lsolve;
  TimeDurationPtr  my_combine    = my_stats_info->combine;
  TimeDurationPtr  my_projection = my_stats_info->projection;
  TransferTimesPtr my_transfer   = my_stats_info->transfer;

  *my_lsolve     = (*my_lsolve)     + (*time_lsolve);
  *my_combine    = (*my_combine)    + (*time_combine);
  *my_projection = (*my_projection) + (*time_projection);
  *my_transfer   = *time_transfer; // copy here

  std::cerr << "Size of my transfer = " << my_transfer->size() << std::endl;

  PTime s_moment = boost::posix_time::microsec_clock::local_time();
  PTimePtr sending_moment(new PTime(s_moment));

  ReturnMessagePtr returning_message(new ReturnMessage(belief_states, sending_moment, sis));

  std::cerr << "Returning message is: " << std::endl << *returning_message << std::endl;

  return returning_message;

#else
  return belief_states;
#endif // DMCS_STATS_INFO
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
