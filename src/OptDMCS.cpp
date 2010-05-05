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
  : BaseDMCS(c, t),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats))
{ }



OptDMCS::~OptDMCS()
{ }


SignaturePtr 
OptDMCS::createGuessingSignature(const BeliefStatePtr& V, const SignaturePtr& my_sig)
{
  SignaturePtr guessing_sig(new Signature);

  // local id in guessing_sig will start from my signature's size + 1
  std::size_t guessing_sig_local_id = my_sig->size() + 1;

  const SignatureBySym& my_sig_sym = boost::get<Tag::Sym>(*my_sig);



  //for (Neighbors::const_iterator n_it = neighbors->begin(); n_it != neighbors->end(); ++n_it)

	std::size_t i = 0;
  for (BeliefState::const_iterator it = V->begin(); it != V->end(); ++it, ++i)
    {
      const BeliefSet neighbor_V = *it;

      if (!isEpsilon(neighbor_V))
				{
#ifdef DEBUG
	  std::cerr << "Interface variable of neighbor[" << i+1 <<"]: " << neighbor_V << std::endl;
#endif
	  
	  const Signature& neighbor_sig = ctx->getQueryPlan()->getSignature(i);

	  guessing_sig_local_id = updateGuessingSignature(guessing_sig,
													  my_sig_sym,
													  neighbor_sig,
													  neighbor_V,
													  guessing_sig_local_id);
				}
    }
      
#ifdef DEBUG
    std::cerr << "Guessing signature: " << *guessing_sig << std::endl;
#endif

	return guessing_sig;
}



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
      TimeDuration local_i(0, 0, 0, 0);
      TimeDuration combine_i(0, 0, 0, 0);
      TimeDuration project_i(0, 0, 0, 0);

      Info info_local(local_i, 0);
      Info info_combine(combine_i, 0);
      Info info_project(project_i, 0);

      TransferTimesPtr tf_i(new TransferTimes);

      StatsInfo si(info_local, info_combine, info_project, tf_i);      

      sis->push_back(si);
    }
  
  TimeDuration time_combine(0, 0, 0, 0);

  std::cerr << "Initialization of the statistic information: " << *sis << std::endl;

  TransferTimesPtr time_transfer(new TransferTimes);
  StatsInfo& my_stats_info = (*sis)[k-1];
#endif


#if defined(DEBUG)
  std::cerr << "OptDMCS at " << k << ", invoker ID " << c << std::endl;
#endif // DEBUG

  // get V from the query plan
  BeliefStatePtr localV(new BeliefState(n, 0));
  //BeliefStatePtr globalV(new BeliefState(n, 0));
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
  //globalV = query_plan->getGlobalV();

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

  ///@todo compute guessing signature and pass it on to localsolve as ProxySignatureByLocalId

  BeliefStateListPtr local_belief_states;

  ///@todo this has to be fixed ........... when Proxy is implelemented
	const SignaturePtr& sig = ctx->getSignature();

	//createGuessingSignature()

    /// ***************** FOR TESTING ****************************

     SignatureIterators insert_iterators;

      
#ifdef DEBUG
    std::cerr << "Original signature: " << *sig << std::endl;
#endif
      
      std::size_t my_id = ctx->getContextID();
      
      const NeighborListPtr& neighbors = query_plan->getNeighbors(my_id);

      for (NeighborList::const_iterator n_it = neighbors->begin();
					 n_it != neighbors->end();
					 ++n_it)
				{

					for (std::size_t j = 1; j <= n; ++j)
						{
							const BeliefSet neighbor_V = query_plan->getInterface(my_id, *n_it)->at(j-1);

							if (!isEpsilon(neighbor_V))
								{

									const Signature neighbor_sig = query_plan->getSignature(j);
									const SignatureByLocal& neighbor_loc = boost::get<Tag::Local>(neighbor_sig);
									
									
									std::cerr << "check neighbor " << j << std::endl
														<< "neighbor_V = " << neighbor_V << std::endl;
									
									// setup local signature for neighbors: this way we can translate
									// SAT models back to belief states in case we do not
									// reference them in the bridge rules
									for (std::size_t i = 1; i <= neighbor_sig.size(); ++i)
										{
											if (testBeliefSet(neighbor_V, i))
												{
													std::cerr << "Bit " << i << "is on" << std::endl;
													SignatureByLocal::const_iterator neighbor_it = neighbor_loc.find(i);
													std::size_t local_id_here = sig->size()+1; // compute new local id for i'th bit
													
													// add new symbol for neighbor
													Symbol sym(neighbor_it->sym, neighbor_it->ctxId, local_id_here, neighbor_it->origId);
													std::pair<Signature::iterator, bool> sp = sig->insert(sym);
													
													std::cerr << "want to insert " << neighbor_it->sym << std::endl;
													
													// only add them if it was not already included
													// during bridge rule parsing
													if (sp.second)
														{
															std::cerr << "insert " << neighbor_it->sym << std::endl;
															insert_iterators.push_back(sp.first);
														}
												}
										}
								}
						}
				}

				
#ifdef DEBUG
      std::cerr << "Updated signature: " << *sig << std::endl;
#endif

    /// **********************************************************

    STATS_DIFF (local_belief_states = localSolve(boost::get<Tag::Local>(*sig)),
		time_lsolve);


    /// ***************** FOR TESTING ****************************
#ifdef DEBUG      
      std::cerr << "Erasing..." << std::endl;
#endif
      
      for (SignatureIterators::const_iterator s_it = insert_iterators.begin();
	   s_it != insert_iterators.end(); 
	   ++s_it)
				{
					sig->erase(*s_it);
				}
      
#ifdef DEBUG
      std::cerr << "Restored signature: " << *sig << std::endl;
#endif
    /// **********************************************************

#ifdef DMCS_STATS_INFO
  my_stats_info.lsolve.second = local_belief_states->size();
#endif // DMCS_STATS_INFO

#ifdef DEBUG
  BeliefStatePtr all_masked(new BeliefState(n, maxBeliefSet()));
  printBeliefStatesNicely(std::cerr, local_belief_states, all_masked, query_plan);
#endif

  ///@todo TK: can we get rid off it?
  BeliefStateListPtr temporary_belief_states(new BeliefStateList);

  //project_to(local_belief_states, globalV, temporary_belief_states);

  STATS_DIFF (project_to(local_belief_states, localV, temporary_belief_states),
	      time_projection);

#ifdef DMCS_STATS_INFO
  my_stats_info.projection.second = temporary_belief_states->size();
#endif // DMCS_STATS_INFO

#if defined(DEBUG)
  std::cerr << "Projected belief states..." << std::endl;
  std::cerr << *temporary_belief_states << std::endl;
  std::cerr << "The V used in projection..." << std::endl;
  std::cerr << *localV << std::endl;

  printBeliefStatesNicely(std::cerr, temporary_belief_states, localV, query_plan);

  std::cerr << "Now check for neighbors..." << std::endl;
#endif // DEBUG

  //
  // now visit the neighbors
  //

  const NeighborListPtr& nbs = query_plan->getNeighbors(k);

#if defined(DEBUG)
  std::cerr << "Number of neighbors: " << nbs->size() << std::endl;
#endif //DEBUG

  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      boost::asio::io_service io_service;
      boost::asio::ip::tcp::resolver resolver(io_service);

      //      std::size_t neighbor_id = *it;
      
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
      
      PTime sent_moment = client_mess->getSendingMoment();
      PTime this_moment = boost::posix_time::microsec_clock::local_time();
      TimeDuration time_neighbor_me = this_moment - sent_moment;
      Info neighbor_transfer(time_neighbor_me, neighbor_belief_states->size());
      
      time_transfer->insert(std::pair<std::size_t, Info>(*it - 1, neighbor_transfer));
#else
      BeliefStateListPtr neighbor_belief_states = client.getResult();
#endif // DMCS_STATS_INFO
      
#if defined(DEBUG)
      std::cerr << "Belief states received from neighbor " << *it << std::endl;	  
      std::cerr << *neighbor_belief_states << std::endl;      
#endif // DEBUG
	  
      //temporary_belief_states = combine(temporary_belief_states, neighbor_belief_states, globalV);

      STATS_DIFF_REUSE (temporary_belief_states = combine(temporary_belief_states,
						neighbor_belief_states,
						localV),
			time_combine
			);

#if defined(DEBUG)
      std::cerr << "Combined belief state:... " << std::endl;	  	  
      std::cerr << *temporary_belief_states << std::endl;
#endif // DEBUG
    }
  
  //  cache->insert(V, belief_states);

#ifdef DMCS_STATS_INFO
  my_stats_info.combine.second = temporary_belief_states->size();
#endif // DMCS_STATS_INFO

  BeliefStateListPtr belief_states(new BeliefStateList);
  project_to(temporary_belief_states, localV, belief_states);

#if defined(DEBUG)
  std::cerr << "Going to send back... " << std::endl;	  	  
  std::cerr << *belief_states << std::endl;
  printBeliefStatesNicely(std::cerr, belief_states,localV, query_plan);
#endif // DEBUG

#ifdef DMCS_STATS_INFO
  my_stats_info.lsolve.first     += time_lsolve;
  my_stats_info.combine.first    += time_combine;
  my_stats_info.projection.first += time_projection;

  TransferTimesPtr my_transfer   = my_stats_info.transfer;

  *my_transfer   = *time_transfer; // copy here

  std::cerr << "Size of my transfer = " << my_transfer->size() << std::endl;

  PTime sending_moment = boost::posix_time::microsec_clock::local_time();

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
