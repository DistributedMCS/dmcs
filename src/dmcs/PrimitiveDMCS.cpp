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

#include "process/ClaspProcess.h"
#include "loopformula/DimacsVisitor.h"

#include "mcs/BeliefState.h"
#include "mcs/ProxySignatureByLocal.h"

#include "dmcs/Cache.h"
#include "dmcs/CommandType.h"
#include "dmcs/BeliefCombination.h"
#include "dmcs/PrimitiveDMCS.h"
#include "dmcs/PrimitiveCommandType.h"
#include "dmcs/QueryPlan.h"

#include "network/Client.h"

#include "parser/BaseBuilder.h"
#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"


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




PrimitiveDMCS::dmcs_return_type
PrimitiveDMCS::getBeliefStates(PrimitiveMessage& mess)
{
  const std::size_t n = ctx->getSystemSize();
  const std::size_t k = ctx->getContextID(); // my local id

  DMCS_LOG_DEBUG("In PrimitiveDMCS, at context " << k);
  DMCS_LOG_DEBUG("n = " << n);

  assert(n > 0 && k <= n);

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
      DMCS_LOG_DEBUG("cache hit");
      belief_states = bs;
      return belief_states;
    }
#endif

    // No cache found, we need to compute from scratch
  
  const SignaturePtr& sig = ctx->getSignature();
  
  DMCS_LOG_DEBUG("Original signature: " << *sig);
      
    // create the guessing signature using global V and my signature
    const SignaturePtr& gsig = createGuessingSignature(V, sig);

    ProxySignatureByLocal mixed_sig(boost::get<Tag::Local>(*sig), boost::get<Tag::Local>(*gsig));

    DMCS_LOG_DEBUG("Local:    " << *sig);
    DMCS_LOG_DEBUG("Guessing: " << *gsig);

    BeliefStateListPtr local_belief_states;

    // This will give us local_belief_states
    STATS_DIFF (local_belief_states = localSolve(mixed_sig, n), time_lsolve);

#ifdef DMCS_STATS_INFO
      my_stats_info.lsolve.second = local_belief_states->size();

      DMCS_LOG_DEBUG("local belief states size: " << my_stats_info.lsolve.second << " == " << local_belief_states->size() << " " << local_belief_states->size());

#endif // DMCS_STATS_INFO

#ifdef DEBUG
      //    BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
#endif // DEBUG

    BeliefStateListPtr belief_states(new BeliefStateList);

    STATS_DIFF (project_to(local_belief_states, V, belief_states),
		time_projection);

#ifdef DMCS_STATS_INFO
    my_stats_info.projection.second = belief_states->size();

    DMCS_LOG_DEBUG("projected belief states size: " << belief_states->size());
    DMCS_LOG_DEBUG(my_stats_info);
#endif // DMCS_STATS_INFO


    DMCS_LOG_DEBUG("Projected belief states:");
    DMCS_LOG_DEBUG(*belief_states);
    DMCS_LOG_DEBUG("The V used in projection:");
    DMCS_LOG_DEBUG(*V);

    DMCS_LOG_DEBUG("Now check for neighbors...");


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
	    DMCS_LOG_DEBUG("Reached a leaf context " << k);
	  }
	else
	  {
	    DMCS_LOG_DEBUG("Cycle detected at context " << k);
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

	DMCS_LOG_DEBUG("At an intermediate context!");

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

	    DMCS_LOG_DEBUG("Invoking neighbor " << neighbor_id);

	    std::string header = HEADER_REQ_PRI_DMCS;
	    Client<PrimitiveCommandType> client(io_service, res_it, header, mess);
	  
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


	    DMCS_LOG_DEBUG("Belief states received from neighbor " << neighbor_id);
	    DMCS_LOG_DEBUG(*neighbor_belief_states);
	    DMCS_LOG_DEBUG("Going to combine " << "k = " << k << " neighbor = " << neighbor_id);


	    STATS_DIFF_REUSE (belief_states = combine(belief_states,
						      neighbor_belief_states,
						      V),
			      time_combine
			      );
	  }

#ifdef DMCS_STATS_INFO
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

	DMCS_LOG_DEBUG("Going to send " << belief_states->size() << " belief states above back to invoker: C_" << invoker);
      }
    else
      {
	DMCS_LOG_DEBUG("Going to send " << belief_states->size() << " belief states above back to user.");
      }
#endif // DEBUG
  
#ifdef DMCS_STATS_INFO
    my_stats_info.lsolve.first     += time_lsolve;
    my_stats_info.combine.first    += time_combine;
    my_stats_info.projection.first += time_projection;

    TransferTimesPtr my_transfer   = my_stats_info.transfer;
    *my_transfer = *time_transfer; // copy here

    PTime sending_moment = boost::posix_time::microsec_clock::local_time();
    ReturnMessagePtr returning_message(new ReturnMessage(belief_states, sending_moment, sis));

    DMCS_LOG_DEBUG("Size of my transfer = " << my_transfer->size());
    DMCS_LOG_DEBUG("Returning message is: ");
    DMCS_LOG_DEBUG(*returning_message);

    return returning_message;
#else
    return belief_states;
#endif // DMCS_STATS_INFO
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
