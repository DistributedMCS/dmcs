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
 * @file   CycleBreaker.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  11 18:30:24 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefStateOffset.h"
#include "mcs/CycleBreaker.h"

namespace dmcs {

CycleBreaker::CycleBreaker(std::size_t ctx_id,
			   std::size_t coff,
			   EvaluatorPtr eval,
			   ReturnPlanMapPtr return_plan,
			   ContextQueryPlanMapPtr queryplan_map,
			   BridgeRuleTablePtr bridge_rules)
  : NewContext(ctx_id, return_plan, queryplan_map, bridge_rules),
    eval(eval)
{
  ctx_offset = coff;
  init();
}


CycleBreaker::~CycleBreaker()
{
  delete total_guessing_input;
  total_guessing_input = NULL;

  delete starting_guess;
  starting_guess = NULL;
}


void
CycleBreaker::init()
{
  std::size_t n = BeliefStateOffset::instance()->NO_BLOCKS();
  std::size_t s = BeliefStateOffset::instance()->SIZE_BS();
  const std::vector<std::size_t>& starting_offsets = BeliefStateOffset::instance()->getStartingOffsets();

  total_guessing_input = new NewBeliefState(n, s);

  starting_guess = new NewBeliefState(n, s);

  for (ContextQueryPlanMap::const_iterator it = queryplan_map->begin(); it != queryplan_map->end(); ++it)
    {
      std::size_t cid = it->first;
      const ContextQueryPlan& cqp = it->second;
      if (cqp.groundInputSignature)
	{
	  total_guessing_input->setEpsilon(cid, starting_offsets);
	  starting_guess->setEpsilon(cid, starting_offsets);

	  std::pair<BeliefTable::AddressIterator, BeliefTable::AddressIterator> iters = cqp.groundInputSignature->getAllByAddress();
	  for (BeliefTable::AddressIterator ait = iters.first; ait != iters.second; ++ait)
	    {
	      const Belief& b = *ait;
	      total_guessing_input->set(cid, b.address, starting_offsets);
	      starting_guess->set(cid, b.address, starting_offsets, NewBeliefState::DMCS_FALSE);
	    }
	}
    }
}


void
CycleBreaker::startup(NewConcurrentMessageDispatcherPtr md,
		      RequestDispatcherPtr rd,
		      NewJoinerDispatcherPtr jd)
{
  int timeout = 0;
  while (1)
    {
      DBGLOG(DBG, "CycleBreaker[" << ctx_id << "]::startup(): Waiting at CYCLE_BREAKER_MQ[" << ctx_offset << "]");
      // Listen to the CYCLE_BREAKER_MQ
      ForwardMessage* fwd_mess = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::CYCLE_BREAKER_MQ, ctx_offset, timeout);
     

      DBGLOG(DBG, "CycleBreaker[" << ctx_id << "]::startup(): Got message: " << *fwd_mess);
      
      std::size_t parent_qid = fwd_mess->qid;
      if (is_shutdown(parent_qid))
	{
	  DBGLOG(DBG, "CycleBreaker[" << ctx_id << "]::startup(): got shut_down request. BREAK NOW!");
	  break;
	}
      
      std::size_t k1 = fwd_mess->k1;
      std::size_t k2 = fwd_mess->k2;

      // Bad requests are not allowed
      assert ((k1 == 0 && k2 == 0) || (0 < k1 && k1 < k2+1));

      // different from non-cycle-breaking task, we will guess for all input here,
      // including the bridge atoms from neighbors that might not be involved in the current cycle.
      NewBeliefState* current_guess = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
							 BeliefStateOffset::instance()->SIZE_BS());
      
      (*current_guess) = (*starting_guess);      

      do
	{
	  if (compute(current_guess, k1, k2, parent_qid, eval, md)) break;
	  current_guess = next_guess(current_guess, total_guessing_input);
	}
      while (current_guess);

      // Send the marker for the end of models
      ReturnedBeliefState* rbs = new ReturnedBeliefState(NULL, parent_qid);
      md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);      
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
