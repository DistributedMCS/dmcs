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
 * @file   OptDMCS.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Nov  18 18:03:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef OPT_DMCS_TCC
#define OPT_DMCS_TCC

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "BeliefCombination.h"
#include "ClaspResultGrammar.h"
#include "ClaspResultBuilder.h"
#include "ClaspProcess.h"
#include "Client.h"
#include "DimacsVisitor.h"
#include "ParserDirector.h"
#include "PrimitiveDMCS.h"
#include "Cache.h"

#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace dmcs {

OptDMCS::OptDMCS(ContextPtr& c, TheoryPtr & t)
  : BaseDMCS(c),
    belief_states(new BeliefStates(c->getSystemSize())),
    local_belief_states(new BeliefStates(c->getSystemSize())),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats)),
    theory(t)
{ }



OptDMCS::~OptDMCS()
{ }



void
OptDMCS::localSolve(const BeliefStatePtr& V)
{
#ifdef DEBUG
  std::cerr << "Starting local solve..." << std::endl;
#endif

  ClaspProcess cp;
  cp.addOption("-n 0");
  boost::shared_ptr<BaseSolver> solver(cp.createSolver());
  std::cerr << "Calling solver..." << std::endl;
  solver->solve(*ctx, local_belief_states, theory, V);


#ifdef DEBUG
  std::cerr << "Got " << local_belief_states.belief_states_ptr->belief_states.size();
  std::cerr << " answers from CLASP" << std::endl;
  std::cerr << "Local belief states from localsolve(): " << std::endl;
  std::cerr << local_belief_states << std::endl;
#endif
}



BeliefStatesPtr
OptDMCS::getBeliefStates(OptMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id
  const std::size_t c = mess.getInvoker();
  belief_states.belief_states_ptr->belief_states.clear();

#ifdef DEBUG
  std::cerr << "invoker ID " << c << std::endl;
#endif

  // get V from the query plan
  BeliefStatePtr localV(new BeliefState(n));
  BeliefStatePtr globalV(new BeliefState(n));
  BeliefStatePtr mask(new BeliefState(n));

  const QueryPlanPtr query_plan = ctx->getQueryPlan();

  // if c is 0, the client invoked this DMCS, thus we don't have an
  // edge in the query plan and use just the maximal V
  if (c == 0) 
    {
      localV.belief_state_ptr->belief_state = BeliefSets(n, std::numeric_limits<unsigned long>::max());
    }
  else // some context invoked this DMCS
    {
      localV = query_plan->getInterface(c, k);
    }

  ///@todo we have to check if we really can use a localized version of the global V
  globalV = query_plan->getGlobalV();

  /*
  for (int i = 0; i < n; ++i)
    {
      if (isEpsilon(localV.belief_state_ptr->belief_state[i]))
	{
	  mask.belief_state_ptr->belief_state[i] = 0;
	}
      else
	{
	  mask.belief_state_ptr->belief_state[i] = std::numeric_limits<unsigned long>::max();
	}
    }

  for (int i = 0; i < n; ++i)
    {
      globalV.belief_state_ptr->belief_state[i] &= mask.belief_state_ptr->belief_state[i];
    }
  */

  std::cerr << c << " calling " << k << std::endl;


  const NeighborsPtr_& nbs = query_plan->getNeighbors(k);

  ///@todo use cache in DMCS
  //  BeliefStatesPtr bs = cache->cacheHit(V);

#if defined(DEBUG)
  std::cerr << "In OptDMCS: ";
#endif


  //  if (bs.belief_states_ptr) 
  //{
#if defined(DEBUG)
  //std::cerr << "cache hit" << std::endl;
#endif //DEBUG

      //      belief_states = bs;
      //return belief_states;
      //    }

  // No cache found, we need to compute from scratch
  // Compute all of our local belief states


  // This will give us local_belief_states
  localSolve(globalV);

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n));
  all_masked.belief_state_ptr->belief_state = BeliefSets(n, std::numeric_limits<unsigned long>::max());
  printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif

  BeliefStatesPtr temporary_belief_states(new BeliefStates(n));

  project_to(local_belief_states, globalV, temporary_belief_states);

#if defined(DEBUG)
  std::cerr << "Projected belief states..." << std::endl;
  std::cerr << belief_states << std::endl;
  std::cerr << "The V used in projection..." << std::endl;
  std::cerr << globalV << std::endl;
  printBeliefStatesNicely(std::cerr, temporary_belief_states, globalV, query_plan);
  std::cerr << "Now check for neighbors..." << std::endl;
#endif

  std::cerr << "Number of neighbors: " << nbs->size() << std::endl;
  for (Neighbors_::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
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
      Client<OptMessage> client(io_service, res_it, n, neighbourMess);
      
      io_service.run();
      
      BeliefStatesPtr bs = client.getBeliefStates();
      
      //BeliefStatesPtr pbs(new BeliefStates(n));
      
      //project_to(bs, ctx->getQueryPlan()->getInterface(k, *it), pbs);
      
#if defined(DEBUG)
      std::cerr << "Belief states received from neighbor " << *it << std::endl;	  
      std::cerr << bs << std::endl;
      //std::cerr << "Projected to interface vars " << V << std::endl;	  
      //std::cerr << pbs << std::endl;
      
#endif // DEBUG
	  
      //belief_states = combine(belief_states, pbs, k, *it, ctx->getQueryPlan()->getInterface(k, *it));
      //belief_states = combine(belief_states, bs, k, *it, ctx->getQueryPlan()->getInterface(k, *it));
      //const BeliefStatePtr& localV = ctx->getQueryPlan()->getInterface(k, *it);

      //std::cerr << "projected global V = " << globalV << std::endl;
      temporary_belief_states = combine(temporary_belief_states, bs, globalV);
      
#if defined(DEBUG)
      std::cerr << "Combined at our belief state:... " << std::endl;	  	  
      std::cerr << temporary_belief_states << std::endl;
#endif // DEBUG
    }
  
  //  cache->insert(V, belief_states);


  project_to(temporary_belief_states, localV, belief_states);

  std::cerr << "Going to send back... " << std::endl;	  	  
  std::cerr << belief_states << std::endl;

  return belief_states;
}

} // namespace dmcs

#endif // OPT_DMCS_TCC

// Local Variables:
// mode: C++
// End:
