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
 * @file   OptDMCS.cpp
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

#include "BeliefState.h"
#include "BeliefCombination.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ClaspProcess.h"
#include "Client.h"
#include "CommandType.h"
#include "DimacsVisitor.h"
#include "ParserDirector.h"
#include "OptDMCS.h"
#include "Cache.h"

#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace dmcs {

OptDMCS::OptDMCS(const ContextPtr& c, const TheoryPtr & t)
  : BaseDMCS(c,t),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats))
{ }



OptDMCS::~OptDMCS()
{ }



OptDMCS::dmcs_return_type
OptDMCS::getBeliefStates(const OptMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id
  const std::size_t c = mess.getInvoker();

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


#if defined(DEBUG)
  std::cerr << "OptDMCS at " << k << ", invoker ID " << c << std::endl;
#endif // DEBUG

  // get V from the query plan
  BeliefStatePtr localV(new BeliefState(n, 0));
  BeliefStatePtr globalV(new BeliefState(n, 0));
  BeliefStatePtr mask(new BeliefState(n, 0));

  const QueryPlanPtr query_plan = ctx->getQueryPlan();

  // if c is 0, the client invoked this DMCS, thus we don't have an
  // edge in the query plan and use just the maximal V
  if (c == 0) 
    {
      BeliefStatePtr nv(new BeliefState(n, maxBeliefSet()));
      localV = nv; // everything set true
    }
  else // some context invoked this DMCS
    {
      localV = query_plan->getInterface(c, k);
    }

  ///@todo we have to check if we really can use a localized version of the global V
  globalV = query_plan->getGlobalV();

#if defined(DEBUG)
  std::cerr << "context " << c << " is calling context " << k << std::endl;
#endif // DEBUG

  ///@todo use cache in DMCS
  //  BeliefStatesPtr bs = cache->cacheHit(V);

  //  if (bs.belief_states_ptr) 
  //{
  //#if defined(DEBUG)
  //std::cerr << "cache hit" << std::endl;
  //#endif //DEBUG

      //      belief_states = bs;
      //return belief_states;
      //    }

  // No cache found, we need to compute from scratch
  // Compute all of our local belief states


  //
  // call the local solver
  //

  // This will give us local_belief_states
  //BeliefStateListPtr local_belief_states = localSolve(globalV);

  BeliefStateListPtr local_belief_states;

  STATS_DIFF (local_belief_states = localSolve(globalV),
	      time_lsolve);

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
  printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif

  ///@todo TK: can we get rid off it?
  BeliefStateListPtr temporary_belief_states(new BeliefStateList);

  //project_to(local_belief_states, globalV, temporary_belief_states);

  STATS_DIFF (project_to(local_belief_states, globalV, temporary_belief_states),
	      time_projection);

#if defined(DEBUG)
  std::cerr << "Projected belief states..." << std::endl;
  std::cerr << *temporary_belief_states << std::endl;
  std::cerr << "The V used in projection..." << std::endl;
  std::cerr << *globalV << std::endl;

  printBeliefStatesNicely(std::cerr, temporary_belief_states, globalV, query_plan);

  std::cerr << "Now check for neighbors..." << std::endl;
#endif // DEBUG

  //
  // now visit the neighbors
  //

  const NeighborsPtr& nbs = query_plan->getNeighbors(k);

#if defined(DEBUG)
  std::cerr << "Number of neighbors: " << nbs->size() << std::endl;
#endif //DEBUG

  for (Neighbors::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      boost::asio::io_service io_service;
      boost::asio::ip::tcp::resolver resolver(io_service);

      std::size_t neighbor_id = *it;
      
      boost::asio::ip::tcp::resolver::query query(query_plan->getHostname(*it),
						  query_plan->getPort(*it));
      boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
      boost::asio::ip::tcp::endpoint endpoint = *res_it;
      
#if defined(DEBUG)
      std::cerr << "Invoking neighbor " << *it << std::endl;
#endif // DEBUG

      OptMessage neighbourMess(k);
      Client<OptCommandType> client(io_service, res_it, neighbourMess);
      
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
      BeliefStateListPtr neighbor_belief_states = client.getResult();
#endif // DMCS_STATS_INFO
      
#if defined(DEBUG)
      //std::cerr << "Belief states received from neighbor " << *it << std::endl;	  
      //std::cerr << neighbor_belief_states << std::endl;      
#endif // DEBUG
	  
      //temporary_belief_states = combine(temporary_belief_states, neighbor_belief_states, globalV);

      STATS_DIFF_REUSE (temporary_belief_states = combine(temporary_belief_states,
						neighbor_belief_states,
						globalV),
			time_combine
			);

#if defined(DEBUG)
      //std::cerr << "Combined belief state:... " << std::endl;	  	  
      //std::cerr << temporary_belief_states << std::endl;
#endif // DEBUG
    }
  
  //  cache->insert(V, belief_states);

  BeliefStateListPtr belief_states(new BeliefStateList);
  project_to(temporary_belief_states, localV, belief_states);

#if defined(DEBUG)
  std::cerr << "Going to send back... " << std::endl;	  	  
  std::cerr << *belief_states << std::endl;
  printBeliefStatesNicely(std::cerr, belief_states,globalV, query_plan);
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
