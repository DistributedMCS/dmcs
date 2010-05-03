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



BeliefStateListPtr
OptDMCS::getBeliefStates(const OptMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id
  const std::size_t c = mess.getInvoker();

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
  BeliefStateListPtr local_belief_states = localSolve(globalV);

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
  printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif

  ///@todo TK: can we get rid off it?
  BeliefStateListPtr temporary_belief_states(new BeliefStateList);

  project_to(local_belief_states, globalV, temporary_belief_states);

#if defined(DEBUG)
  std::cerr << "Projected belief states..." << std::endl;
  std::cerr << temporary_belief_states << std::endl;
  std::cerr << "The V used in projection..." << std::endl;
  std::cerr << globalV << std::endl;

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
      
      BeliefStateListPtr neighbor_belief_states = client.getResult();
      
#if defined(DEBUG)
      std::cerr << "Belief states received from neighbor " << *it << std::endl;	  
      std::cerr << neighbor_belief_states << std::endl;      
#endif // DEBUG
	  
      temporary_belief_states = combine(temporary_belief_states,
					neighbor_belief_states,
					globalV);
      
#if defined(DEBUG)
      std::cerr << "Combined belief state:... " << std::endl;	  	  
      std::cerr << temporary_belief_states << std::endl;
#endif // DEBUG
    }
  
  //  cache->insert(V, belief_states);

  BeliefStateListPtr belief_states(new BeliefStateList);
  project_to(temporary_belief_states, localV, belief_states);

#if defined(DEBUG)
  std::cerr << "Going to send back... " << std::endl;	  	  
  std::cerr << belief_states << std::endl;
#endif // DEBUG

  return belief_states;
}

} // namespace dmcs


// Local Variables:
// mode: C++
// End:
