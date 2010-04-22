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
 * @file   PrimitiveDMCS.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Nov  18 18:03:24 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef PRIMITIVE_DMCS_TCC
#define PRIMITIVE_DMCS_TCC

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

#include "QueryPlan.h"
#include "BeliefState.h"

#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace dmcs {


PrimitiveDMCS::PrimitiveDMCS(ContextPtr& c, TheoryPtr & t)
  : BaseDMCS(c),
    belief_states(new BeliefStates(c->getSystemSize())),
    local_belief_states(new BeliefStates(c->getSystemSize())),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats)),
    theory(t)
{ }



PrimitiveDMCS::~PrimitiveDMCS()
{ }



void
PrimitiveDMCS::localSolve(const BeliefStatePtr& V)
{
#ifdef DEBUG
  std::cerr << "Starting local solve..." << std::endl;
#endif

  ClaspProcess cp;
  cp.addOption("-n 0");
  boost::shared_ptr<BaseSolver> solver(cp.createSolver());

  solver->solve(*ctx, local_belief_states, theory, V);

#ifdef DEBUG
  std::cerr << "Got " << local_belief_states.belief_states_ptr->belief_states.size();
  std::cerr << " answers from CLASP" << std::endl;
  std::cerr << "Local belief states from localsolve(): " << std::endl;
  std::cerr << local_belief_states << std::endl;
#endif
}



BeliefStatesPtr
PrimitiveDMCS::getBeliefStates(PrimitiveMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id

  assert(n > 0 && k <= n);

  belief_states.belief_states_ptr->belief_states.clear();

  const QueryPlanPtr query_plan = ctx->getQueryPlan();

  const BeliefStatePtr& V = mess.getV();

#if defined(DEBUG)
  std::cerr << "In PrimitiveDMCS, at context " << k << std::endl;
#endif

#if 0
  BeliefStatesPtr bs = cache->cacheHit(V);

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

  localSolve(V);

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n));
  all_masked.belief_state_ptr->belief_state = BeliefSets(n, std::numeric_limits<unsigned long>::max());
  printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif // DEBUG

  project_to(local_belief_states, V, belief_states);

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
  const NeighborsPtr_& nbs = query_plan->getNeighbors(k);
  
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
      for (Neighbors_::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
	{
	  boost::asio::io_service io_service;
	  boost::asio::ip::tcp::resolver resolver(io_service);

	  boost::asio::ip::tcp::resolver::query query(ctx->getQueryPlan()->getHostname(*it),
						      ctx->getQueryPlan()->getPort(*it));
	  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
	  boost::asio::ip::tcp::endpoint endpoint = *res_it;
	  
	  mess.insertHistory(k);

#if defined(DEBUG)
	  std::cerr << "Invoking neighbor " << *it << std::endl;
#endif // DEBUG

	  Client<PrimitiveMessage> client(io_service, res_it, n, mess);
	  
	  io_service.run();

	  BeliefStatesPtr bs = client.getBeliefStates();
	  mess.removeHistory(k);

#if defined(DEBUG)
	  std::cerr << "Belief states received from neighbor " << *it << std::endl;	  
	  std::cerr << bs << std::endl;
	  std::cerr << "Going to combine " << "k = " << k << " neighbor = " << *it << std::endl;
#endif // DEBUG

	  belief_states = combine(belief_states, bs, V);

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
      std::cerr << "Going to send " << belief_states.belief_states_ptr->belief_states.size()
		<< " belief states above back to invoker: C_" << invoker << std::endl;
    }
  else
    {
      std::cerr << "Going to send " << belief_states.belief_states_ptr->belief_states.size()
		<< " belief states above back to user." << std::endl;
    }
#endif

#ifdef DEBUG
  printBeliefStatesNicely(std::cerr, belief_states, V, query_plan);
#endif
  
  return belief_states;
}

} // namespace dmcs

#endif // PRIMITIVE_DMCS_TCC

// Local Variables:
// mode: C++
// End:
