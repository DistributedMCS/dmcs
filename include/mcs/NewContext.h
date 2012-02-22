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
 * @file   NewContext.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  21 17:43:20 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_CONTEXT_H
#define NEW_CONTEXT_H

#include "dmcs/Instantiator.h"
#include "mcs/BeliefTable.h"
#include "mcs/BridgeRuleEvaluator.h"
#include "mcs/StreamingJoiner.h"
#include "network/NewConcurrentMessageDispatcher.h"

namespace dmcs {

class NewContext 
{
public:
  // For now, we initialize the context only at the beginning.
  // Later, the idea is to have the query plan included in the request to a context.
  // Upon receiving a request, the context goes to the specified query plan and 
  // reads off information regarding bridge rules, neighbors, export signature,...
  // This is the dynamic setting for future work.
  NewContext(std::size_t cid,
	     InstantiatorPtr i,
	     BeliefTablePtr ex_sig,
	     BridgeRuleTablePtr br,
	     NewNeighborVecPtr nbs,
	     NeighborOffset2IndexPtr o2i);

  // Constructor for leaf contexts
  NewContext(std::size_t cid,
	     InstantiatorPtr i,
	     BeliefTablePtr ex_sig);

  void
  operator()(NewConcurrentMessageDispatcherPtr md,
	     NewJoinerDispatcherPtr jd);
  

private:
  bool
  read_until_k2(std::size_t k1, 
		std::size_t k2,
		std::size_t parent_qid,
		EvaluatorPtr eval,
		NewConcurrentMessageDispatcherPtr md);

  void
  read_all(std::size_t parent_qid,
	   EvaluatorPtr eval,
	   NewConcurrentMessageDispatcherPtr md);

  void
  send_out_result(std::size_t parent_qid,
		  Heads* heads,
		  NewBeliefState* belief_state,
		  NewConcurrentMessageDispatcherPtr md);

  void
  reset();

private:
  bool is_leaf;
  std::size_t ctx_id;
  std::size_t query_counter;
  std::size_t answer_counter;

  // The instantiator holds the path (string) to the local knowledge base
  InstantiatorPtr inst;

  BridgeRuleTablePtr bridge_rules;

  // This just contains the beliefs that are exported to the parents.
  // Other local beliefs should be hidden inside the Evaluator and treated
  // as text. 
  BeliefTablePtr export_signature;

  // For now, neighbors' signatures are not needed
  // as we assume that they are provided once by the query plan.
  NewNeighborVecPtr neighbors;

  NeighborOffset2IndexPtr offset2index;

  StreamingJoinerPtr joiner;
};

} // namespace dmcs

#endif // NEW_CONTEXT_H
  
// Local Variables:
// mode: C++
// End:
