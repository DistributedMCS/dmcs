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
#include "mcs/QueryPlan.h"
#include "mcs/RequestDispatcher.h"
#include "mcs/StreamingJoiner.h"
#include "network/NewConcurrentMessageDispatcher.h"

#include <boost/shared_ptr.hpp>

namespace dmcs {

class NewContext 
{
public:
  // For now, we initialize the context only at the beginning.
  // Later, the idea is to have the query plan included in the request to a context.
  // Upon receiving a request, the context goes to the specified query plan and 
  // reads off information regarding bridge rules, neighbors, local signature,...
  // This is the dynamic setting for future work.
  NewContext(std::size_t cid,
	     ReturnPlanMapPtr return_plan,
	     ContextQueryPlanMapPtr queryplan_map,
	     BridgeRuleTablePtr br);
  
  ~NewContext();

  virtual void
  startup(NewConcurrentMessageDispatcherPtr md,
	  RequestDispatcherPtr rd,
	  NewJoinerDispatcherPtr jd = NewJoinerDispatcherPtr()) = 0;

protected:
  virtual void
  init() = 0;

  NewBeliefState*
  next_guess(NewBeliefState* current_guess,
	     NewBeliefState* guessing_input);
  
  bool
  compute(NewBeliefState* input,
	  std::size_t& k1,
	  std::size_t& k2,
	  std::size_t parent_qid,
	  EvaluatorPtr eval,
	  NewConcurrentMessageDispatcherPtr md);

  std::size_t
  read_and_send(std::size_t parent_qid,
		bool normal_solve,
		EvaluatorPtr eval,
		NewConcurrentMessageDispatcherPtr md);

  bool
  read_and_send_k1_k2(std::size_t parent_qid,
		      bool normal_solve,
		      std::size_t& k1,
		      std::size_t& k2,
		      EvaluatorPtr eval,
		      NewConcurrentMessageDispatcherPtr md);

  bool
  send_out_result(std::size_t parent_qid,
		  bool normal_solve,
		  Heads* heads,
		  NewBeliefState* belief_state,
		  NewConcurrentMessageDispatcherPtr md);

protected:
  std::size_t ctx_id;
  std::size_t ctx_offset;
  ReturnPlanMapPtr return_plan;
  ContextQueryPlanMapPtr queryplan_map;
  BridgeRuleTablePtr bridge_rules;
  NewBeliefState* total_guessing_input;
  NewBeliefState* starting_guess;
};

typedef boost::shared_ptr<NewContext> NewContextPtr;
typedef std::vector<NewContextPtr> NewContextVec;
typedef boost::shared_ptr<NewContextVec> NewContextVecPtr;

} // namespace dmcs

#endif // NEW_CONTEXT_H
  
// Local Variables:
// mode: C++
// End:
