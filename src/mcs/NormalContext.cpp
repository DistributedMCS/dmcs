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
 * @file   NormalContext.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  11 23:04:26 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefStateOffset.h"
#include "mcs/NormalContext.h"
#include "mcs/CycleBreaker.h"
#include "mcs/ForwardMessage.h"
#include "mcs/LeafJoiner.h"
#include "mcs/StreamingJoiner.h"
#include "dmcs/DLVEvaluator.h"


namespace dmcs {

NormalContext::NormalContext(std::size_t cid,
		       std::size_t pack_size,
		       InstantiatorPtr i,
		       BeliefTablePtr lsig,
		       ReturnPlanMapPtr return_plan,
		       ContextQueryPlanMapPtr queryplan_map,
		       BridgeRuleTablePtr br,
		       NewNeighborVecPtr nbs,
		       NewNeighborVecPtr gnbs)
  : NewContext(cid, return_plan, queryplan_map, br),
    is_leaf(nbs->size() == 0),
    query_counter(0),
    inst(i),
    local_signature(lsig),
    neighbors(nbs),
    guessing_neighbors(gnbs)
{
  if (nbs->size() == 0)
    joiner = LeafJoinerPtr(new LeafJoiner(nbs));
  else
    joiner = StreamingJoinerPtr(new StreamingJoiner(pack_size, nbs));

  init();
}


NormalContext::~NormalContext()
{
  if (total_guessing_input)
    {
      delete total_guessing_input;
      total_guessing_input = NULL;
    }

  if (starting_guess)
    {
      delete starting_guess;
      starting_guess = NULL;
    }
}



void
NormalContext::init()
{
  std::size_t n = BeliefStateOffset::instance()->NO_BLOCKS();
  std::size_t s = BeliefStateOffset::instance()->SIZE_BS();
  const std::vector<std::size_t>& starting_offsets =  BeliefStateOffset::instance()->getStartingOffsets();

  // compute total_guessing_input once and for all
  if (guessing_neighbors == NewNeighborVecPtr())
    {
      total_guessing_input = NULL;
      starting_guess = NULL;
    }
  else
    {
      total_guessing_input = new NewBeliefState(n, s);
      starting_guess = new NewBeliefState(n, s);

      for (NewNeighborVec::const_iterator it = guessing_neighbors->begin(); it != guessing_neighbors->end(); ++it)
	{
	  NewNeighborPtr neighbor = *it;
	  std::size_t nid = neighbor->neighbor_id;

	  total_guessing_input->setEpsilon(nid, starting_offsets);
	  starting_guess->setEpsilon(nid, starting_offsets);

	  ContextQueryPlanMap::const_iterator qit = queryplan_map->find(nid);
	  assert (qit != queryplan_map->end());
	  const ContextQueryPlan& cqp = qit->second;

	  if (cqp.groundInputSignature)
	    {
	      std::pair<BeliefTable::AddressIterator, BeliefTable::AddressIterator> iters = cqp.groundInputSignature->getAllByAddress();
	      for (BeliefTable::AddressIterator ait = iters.first; ait != iters.second; ++ait)
		{
		  const Belief& b = *ait;
		  total_guessing_input->set(nid, b.address, starting_offsets);
		  starting_guess->set(nid, b.address, starting_offsets, NewBeliefState::DMCS_FALSE);
		}
	    }
	}
    }
}


std::size_t
NormalContext::getRequestOffset()
{
  return ctx_offset;
}



NewNeighborVecPtr
NormalContext::getNeighbors()
{
  return neighbors;
}


void
NormalContext::startup(NewConcurrentMessageDispatcherPtr md,
		    RequestDispatcherPtr rd,
		    NewJoinerDispatcherPtr jd)
{
  // Register REQUESTMQ to md
  ctx_offset = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::REQUEST_MQ);
  rd->registerIdOffset(ctx_id, ctx_offset);

  md->createAndRegisterMQ(NewConcurrentMessageDispatcher::CYCLE_BREAKER_MQ, ctx_offset);

  if (!is_leaf)
    {
      assert (joiner);
      joiner->registerJoinIn(ctx_offset, md);
    }

  // Start evaluator thread
  InstantiatorWPtr inst_wptr(inst);
  EvaluatorPtr eval = inst->createEvaluator(inst_wptr);
  inst->startThread(eval, ctx_id, local_signature, md);

  // spawn a corresponding cycle breaker
  CycleBreakerPtr cycle_breaker(new CycleBreaker(ctx_id, ctx_offset, eval, 
						 return_plan, queryplan_map, bridge_rules));
  CycleBreakerWrapper cycle_breaker_wrapper;
  cycle_breaker_thread = new boost::thread(cycle_breaker_wrapper, cycle_breaker, md, rd, jd);

  int timeout = 0;
  while (1)
    {
      DBGLOG(DBG, "NormalContext[" << ctx_id << "]::startup(): Waiting at REQUEST_MQ[" << ctx_offset << "]");
      // Listen to the REQUEST_MQ
      ForwardMessage* fwd_mess = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_offset, timeout);

      DBGLOG(DBG, "NormalContext[" << ctx_id << "]::startup(): Got message: " << *fwd_mess);
      
      std::size_t parent_qid = fwd_mess->qid;
      if (is_shutdown(parent_qid))
	{
	  DBGLOG(DBG, "NormalContext[" << ctx_id << "]::startup(): got shut_down request. BREAK NOW!");
	  break;
	}

      std::size_t k1 = fwd_mess->k1;
      std::size_t k2 = fwd_mess->k2;

      // Bad requests are not allowed
      assert ((k1 == 0 && k2 == 0) || (0 < k1 && k1 < k2+1));

      // cycle detecting is handled at RequestDispatcher. 
      // If it is detected, then a thread for cycle breaking will be created to deal with the situation.
      // We don't have to care about breaking the cycles here.

      NewHistory history = fwd_mess->history;
      assert (history.find(ctx_id) == history.end());
      history.insert(ctx_id);
      process_request(parent_qid, history, eval, md, jd, k1, k2);

      // Send the marker for the end of models
      ReturnedBeliefState* rbs = new ReturnedBeliefState(NULL, parent_qid);
      md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);      
    } 

  // send NULL to evaluator to tell it to stop
  Heads* end_heads = NULL;
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), end_heads, timeout);
  inst->stopThread(eval);
}



bool
NormalContext::process_input(NewBeliefState* input,
			     std::size_t parent_qid,
			     EvaluatorPtr eval,
			     NewConcurrentMessageDispatcherPtr md,
			     std::size_t& k1,
			     std::size_t& k2)
{
  if (total_guessing_input != NULL)
    {
      NewBeliefState* current_guess = new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
							 BeliefStateOffset::instance()->SIZE_BS());
      
      (*current_guess) = (*starting_guess);
      
      // iterate over all possible guesses or until k1 --> k2 models were computed
      DBGLOG(DBG, "NormalContext::process_request(). Guessing input = " << *total_guessing_input);
      bool computed_k1_k2 = false;
      do
	{
	  NewBeliefState* combined_input = 
	    new NewBeliefState(BeliefStateOffset::instance()->NO_BLOCKS(),
			       BeliefStateOffset::instance()->SIZE_BS());
	  (*combined_input ) = (*input) | (*current_guess);
	  
	  if (compute(combined_input, k1, k2, parent_qid, eval, md)) 
	    {
	      computed_k1_k2 = true;
	      break;
	    }
	  
	  current_guess = next_guess(current_guess, total_guessing_input);
	}
      while (current_guess);

      // carefully clean up
      if (current_guess)
	{
	  delete current_guess;
	  current_guess = NULL;
	}

      return computed_k1_k2;
    }
  else
    {
      return compute(input, k1, k2, parent_qid, eval, md);
    }
}



void
NormalContext::process_request(std::size_t parent_qid,
			       const NewHistory& history,
			       EvaluatorPtr eval,
			       NewConcurrentMessageDispatcherPtr md,
			       NewJoinerDispatcherPtr jd,
			       std::size_t k1,
			       std::size_t k2)
{
  assert ((k1 == 0 && k2 == 0) || (0 < k1 && k1 < k2+1));

  std::size_t this_qid = query_id(ctx_id, ++query_counter);
  ReturnedBeliefState* rbs = joiner->first_join(this_qid, history, md, jd);
  NewBeliefState* input = rbs->belief_state;

  while (input != NULL)
    {
      DBGLOG(DBG, "NormalContext::process_request: input = " << *input);
      if (process_input(input, parent_qid, eval, md, k1, k2)) break;

      this_qid = query_id(ctx_id, ++query_counter);
      rbs = joiner->next_join(this_qid, history, md, jd);
      input = rbs->belief_state;
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
