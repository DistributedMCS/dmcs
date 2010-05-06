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
#include "BaseBuilder.h"
#include "ParserDirector.h"
#include "ProxySignatureByLocal.h"


#include <vector>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace dmcs {

PrimitiveDMCS::PrimitiveDMCS(const ContextPtr& c, const TheoryPtr& t, const SignatureVecPtr& s)
  : BaseDMCS(c, t, s),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats))
{ }



PrimitiveDMCS::~PrimitiveDMCS()
{ }



SignaturePtr
PrimitiveDMCS::createGuessingSignature(const BeliefStatePtr& V, const SignaturePtr& my_sig)
{
  SignaturePtr guessing_sig(new Signature);

  // local id in guessing_sig will start from my signature's size + 1
  std::size_t guessing_sig_local_id = my_sig->size() + 1;

  const SignatureBySym& my_sig_sym = boost::get<Tag::Sym>(*my_sig);

  const NeighborListPtr& neighbors = ctx->getNeighbors();

  for (NeighborList::const_iterator n_it = neighbors->begin(); n_it != neighbors->end(); ++n_it)
    {
      NeighborPtr nb = *n_it;
      const std::size_t neighbor_id = nb->neighbor_id;
      const BeliefSet neighbor_V = (*V)[neighbor_id - 1];
      const Signature& neighbor_sig = *((*global_sigs)[neighbor_id - 1]);

#ifdef DEBUG
      std::cerr << "Interface variable of neighbor[" << nb->neighbor_id <<"]: " << neighbor_V << std::endl;
#endif

      guessing_sig_local_id = updateGuessingSignature(guessing_sig,
						      my_sig_sym,
						      neighbor_sig,
						      neighbor_V,
						      guessing_sig_local_id);
    }
      
#ifdef DEBUG
    std::cerr << "Guessing signature: " << *guessing_sig << std::endl;
#endif

    return guessing_sig;
}



PrimitiveDMCS::dmcs_return_type
PrimitiveDMCS::getBeliefStates(PrimitiveMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id
  
  assert(n > 0 && k <= n);
    
#if defined(DEBUG)
  std::cerr << "In PrimitiveDMCS, at context " << k << "n = " << n << std::endl;
#endif // DEBUG

#ifdef DMCS_STATS_INFO
  initStatsInfos(n);

  TimeDuration time_combine(0, 0, 0, 0);
  TransferTimesPtr time_transfer(new TransferTimes);
  StatsInfo& my_stats_info = (*sis)[k-1];
#endif

  const BeliefStatePtr& V = mess.getV();

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

    const SignaturePtr& sig = ctx->getSignature();

#ifdef DEBUG
    std::cerr << "Original signature: " << *sig << std::endl;
#endif
      
    // create the guessing signature using global V and my signature
    const SignaturePtr& gsig = createGuessingSignature(V, sig);

    ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*gsig));

#ifdef DEBUG
      std::cerr << "Local:    " << *sig << std::endl;
      std::cerr << "Guessing: " << *gsig << std::endl;
#endif

    BeliefStateListPtr local_belief_states;

    // This will give us local_belief_states
    STATS_DIFF (local_belief_states = localSolve(mixed_sig, n), time_lsolve);

#ifdef DMCS_STATS_INFO
      my_stats_info.lsolve.second = local_belief_states->size();

#ifdef DEBUG
      std::cerr << "local belief states size: " << my_stats_info.lsolve.second << " == " << local_belief_states->size() << std::endl;
      std::cerr << local_belief_states->size() << std::endl;
#endif // DEBUG


#endif // DMCS_STATS_INFO

#ifdef DEBUG
    BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
    //printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif // DEBUG

    BeliefStateListPtr belief_states(new BeliefStateList);

    STATS_DIFF (project_to(local_belief_states, V, belief_states),
		time_projection);

#ifdef DMCS_STATS_INFO
    my_stats_info.projection.second = belief_states->size();

#ifdef DEBUG
    std::cerr << "projected belief states size: " << belief_states->size() << std::endl;
    std::cerr << my_stats_info << std::endl;
#endif // DEBUG

#endif // DMCS_STATS_INFO


#if defined(DEBUG)
    std::cerr << "Projected belief states..." << std::endl;
    std::cerr << *belief_states << std::endl;
    std::cerr << "The V used in projection..." << std::endl;
    std::cerr << *V << std::endl;
    //printBeliefStatesNicely(std::cerr, belief_states, V, query_plan);
    std::cerr << "Now check for neighbors..." << std::endl;
#endif // DEBUG


    //
    // detect cycles
    //
    const NeighborListPtr& nbs = ctx->getNeighbors();
    const History& hist = mess.getHistory();

    if ((std::find(hist.begin(), hist.end(), k) != hist.end()) || nbs->empty())
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

#ifdef DEBUG
	std::cerr << "At an intermediate context! " << std::endl;
#endif

	for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
	  {
	    boost::asio::io_service io_service;
	    boost::asio::ip::tcp::resolver resolver(io_service);

	    NeighborPtr nb = *it;
	    std::size_t neighbor_id = nb->neighbor_id;

	    // to remove dependency to query plan: give hostname and port to neighbors

	    boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
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

	    PTime sent_moment = client_mess->getSendingMoment();
	    PTime this_moment = boost::posix_time::microsec_clock::local_time();
	    TimeDuration time_neighbor_me = this_moment - sent_moment;
	    Info neighbor_transfer(time_neighbor_me, neighbor_belief_states->size());

	    time_transfer->insert(std::pair<std::size_t, Info>(neighbor_id, neighbor_transfer));
#else
	    BeliefStateListPtr neighbor_belief_states = client_mess;
#endif // DMCS_STATS_INFO


	    mess.removeHistory();


#if defined(DEBUG)
	    std::cerr << "Belief states received from neighbor " << neighbor_id << std::endl;	  
	    std::cerr << *neighbor_belief_states << std::endl;
	    std::cerr << "Going to combine " << "k = " << k << " neighbor = " << neighbor_id << std::endl;
#endif // DEBUG


	    STATS_DIFF_REUSE (belief_states = combine(belief_states,
						      neighbor_belief_states,
						      V),
			      time_combine
			      );

#if defined(DEBUG)
	    std::cerr << "Accumulated combination... " << std::endl;	  	  
	    //std::cerr << *belief_states << std::endl;
#endif // DEBUG
	  }

#ifdef DMCS_STATS_INFO

#ifdef DEBUG
	//std::cerr << "combination size: " << belief_states->size() << std::endl;
#endif // DEBUG

	my_stats_info.combine.second = belief_states->size();
#endif // DMCS_STATS_INFO
      
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
 
    //printBeliefStatesNicely(std::cerr, belief_states, V, query_plan);
#endif // DEBUG
  
#ifdef DMCS_STATS_INFO
    my_stats_info.lsolve.first     += time_lsolve;
    my_stats_info.combine.first    += time_combine;
    my_stats_info.projection.first += time_projection;

    TransferTimesPtr my_transfer   = my_stats_info.transfer;
    *my_transfer = *time_transfer; // copy here

    PTime sending_moment = boost::posix_time::microsec_clock::local_time();
    ReturnMessagePtr returning_message(new ReturnMessage(belief_states, sending_moment, sis));

#ifdef DEBUG
    std::cerr << "Size of my transfer = " << my_transfer->size() << std::endl;
    std::cerr << "Returning message is: " << std::endl << *returning_message << std::endl;
#endif // DEBUG

    return returning_message;

#else
    return belief_states;
#endif // DMCS_STATS_INFO
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
