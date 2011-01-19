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

#include "mcs/BeliefState.h"
#include "dmcs/BeliefCombination.h"
#include "dmcs/Cache.h"
#include "dmcs/CommandType.h"
#include "process/ClaspProcess.h"
#include "loopformula/DimacsVisitor.h"

#include "dmcs/OptDMCS.h"
#include "dmcs/OptCommandType.h"

#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include "network/Client.h"

#if defined(DEBUG)
#include "dmcs/Debug.h"
#endif // DEBUG

#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace dmcs {

  OptDMCS::OptDMCS(const ContextPtr& c, const TheoryPtr& t, const SignatureVecPtr& s, const QueryPlanPtr& query_plan_)
    : BaseDMCS(c, t, s),
    query_plan(query_plan_),
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
  initStatsInfos(n);
  
  TimeDuration time_combine(0, 0, 0, 0);
  TransferTimesPtr time_transfer(new TransferTimes);
  StatsInfo& my_stats_info = (*sis)[k-1];
#endif //DMCS_STATS_INFO


  DMCS_LOG_DEBUG("OptDMCS at " << k << ", invoker ID " << c);

  BeliefStatePtr mask(new BeliefState(n, 0));

  DMCS_LOG_DEBUG("context " << c << " is calling context " << k);

  // get V from the query plan
  BeliefStatePtr localV;

  // if c is 0, the client invoked this DMCS, thus we don't have an
  // edge in the query plan and use just the maximal V
  if (c == 0) 
    {
      //      BeliefStatePtr nv(new BeliefState(n, maxBeliefSet()));
      // localV = nv; // everything set true
      /// just a quick hack here. for a correct implementation, we
      /// need to collect all interfaces of our neighbors.
      localV = query_plan->getGlobalV();
    }
  else // some context invoked this DMCS
    {
      localV = query_plan->getInterface(c, k);
    }

#if 0
  ///@todo use cache in DMCS
  BeliefStatesPtr bs = cache->cacheHit(V);

  if (bs.belief_states_ptr) 
    {
      DMCS_LOG_DEBUG("cache hit");      
      belief_states = bs;
      return belief_states;
    }
#endif //0

  // No cache found, we need to compute from scratch
  // Compute all of our local belief states


  //
  // call the local solver
  //

  BeliefStateListPtr local_belief_states;

  const SignaturePtr& sig = ctx->getSignature();

  DMCS_LOG_DEBUG("Original signature: " << *sig);
      
  const SignaturePtr& gsig = createGuessingSignature(localV, sig);
  
  ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*gsig));

  STATS_DIFF (local_belief_states = localSolve(mixed_sig, query_plan->getSystemSize()), time_lsolve);

#ifdef DMCS_STATS_INFO
  my_stats_info.lsolve.second = local_belief_states->size();
#endif // DMCS_STATS_INFO

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
  //printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif

  ///@todo TK: can we get rid off it?
  BeliefStateListPtr temporary_belief_states(new BeliefStateList);

  //project_to(local_belief_states, globalV, temporary_belief_states);

  STATS_DIFF (project_to(local_belief_states, localV, temporary_belief_states),
	      time_projection);

#ifdef DMCS_STATS_INFO
  my_stats_info.projection.second = temporary_belief_states->size();
#endif // DMCS_STATS_INFO

  DMCS_LOG_DEBUG("Projected belief states...");
  DMCS_LOG_DEBUG(*temporary_belief_states);
  DMCS_LOG_DEBUG("The V used in projection...");
  DMCS_LOG_DEBUG(*localV);

  //printBeliefStatesNicely(std::cerr, temporary_belief_states, localV, query_plan);
  DMCS_LOG_DEBUG( "Now check for neighbors...");

  //
  // now visit the neighbors
  //

  std::size_t my_id = ctx->getContextID();
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);

  DMCS_LOG_DEBUG("Number of neighbors: " << nbs->size());

  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      boost::asio::io_service io_service;
      boost::asio::ip::tcp::resolver resolver(io_service);

      NeighborPtr nb = *it;

      DMCS_LOG_DEBUG("Invoking neighbor " << nb->neighbor_id << "@" << nb->hostname << ":" << nb->port);

      
      boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);

      boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
      boost::asio::ip::tcp::endpoint endpoint = *res_it;
      
      std::string header = HEADER_REQ_OPT_DMCS;
      OptMessage neighbourMess(k);

      Client<OptCommandType> client(io_service, res_it, header, neighbourMess);
      
      io_service.run();


      dmcs_return_type client_mess = client.getResult();

#ifdef DMCS_STATS_INFO
      BeliefStateListPtr neighbor_belief_states = client_mess->getBeliefStates();
      StatsInfosPtr stats_infos = client_mess->getStatsInfo();

      combine(sis, stats_infos);
      
      PTime sent_moment = client_mess->getSendingMoment();
      PTime this_moment = boost::posix_time::microsec_clock::local_time();
      TimeDuration time_neighbor_me = this_moment - sent_moment;
      Info neighbor_transfer(time_neighbor_me, neighbor_belief_states->size());

      time_transfer->insert(std::pair<std::size_t, Info>(nb->neighbor_id - 1, neighbor_transfer));
#else
      BeliefStateListPtr neighbor_belief_states = client.getResult();
#endif // DMCS_STATS_INFO


      DMCS_LOG_DEBUG("Belief states received from neighbor " << *it);
      DMCS_LOG_DEBUG(*neighbor_belief_states);
    
      /// for combination, we need to use Edge_V from myself to the neighbor

      const BeliefStatePtr neighbor_V = query_plan->getInterface(k, nb->neighbor_id);

      STATS_DIFF_REUSE (temporary_belief_states = combine(temporary_belief_states,
							  neighbor_belief_states,
							  neighbor_V),
			time_combine
			);
      
      DMCS_LOG_DEBUG("Combined belief state:... ");
      DMCS_LOG_DEBUG(*temporary_belief_states);
    }
  
  //  cache->insert(V, belief_states);

#ifdef DMCS_STATS_INFO
  my_stats_info.combine.second = temporary_belief_states->size();
#endif // DMCS_STATS_INFO

  BeliefStateListPtr belief_states(new BeliefStateList);
  project_to(temporary_belief_states, localV, belief_states);

  DMCS_LOG_DEBUG("Going to send back...");
  DMCS_LOG_DEBUG(*belief_states);
  //printBeliefStatesNicely(std::cerr, belief_states,localV, query_plan);

#ifdef DMCS_STATS_INFO
  my_stats_info.lsolve.first     += time_lsolve;
  my_stats_info.combine.first    += time_combine;
  my_stats_info.projection.first += time_projection;

  TransferTimesPtr my_transfer   = my_stats_info.transfer;
  *my_transfer   = *time_transfer; // copy here

  PTime sending_moment = boost::posix_time::microsec_clock::local_time();
  ReturnMessagePtr returning_message(new ReturnMessage(belief_states, sending_moment, sis));

  DMCS_LOG_DEBUG("Size of my transfer = " << my_transfer->size());
  DMCS_LOG_DEBUG("Returning message is: " << std::endl << *returning_message);

  return returning_message;
#else
  return belief_states;
#endif // DMCS_STATS_INFO

}			
				

} // namespace dmcs


// Local Variables:
// mode: C++
// End:
