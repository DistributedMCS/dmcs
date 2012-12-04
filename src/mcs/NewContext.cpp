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
 * @file   NewContext.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Feb  16 20:57:26 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefStateOffset.h"
#include "mcs/NewContext.h"
#include "mcs/CycleBreaker.h"
#include "mcs/ForwardMessage.h"
#include "dmcs/DLVEvaluator.h"

namespace dmcs {

NewContext::NewContext(std::size_t cid,
		       ReturnPlanMapPtr return_plan,
		       ContextQueryPlanMapPtr queryplan_map,
		       BridgeRuleTablePtr br)
  : ctx_id(cid),
    return_plan(return_plan),
    queryplan_map(queryplan_map),
    bridge_rules(br)
{ }


NewContext::~NewContext()
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



bool
NewContext::compute(NewBeliefState* input,
		    std::size_t& k1,
		    std::size_t& k2,
		    std::size_t parent_qid,
		    EvaluatorPtr eval,
		    NewConcurrentMessageDispatcherPtr md)
{
  ///@todo: create heads = (NULL, k1, k2) in case of real leaf node.
  DBGLOG(DBG, "NewContext[" << ctx_id << "]::compute(): input = " << *input);
  Heads* heads = evaluate_bridge_rules(bridge_rules, input, k1, k2,
				       BeliefStateOffset::instance()->getStartingOffsets());      
  
  if (heads == NULL)
    {
      return false;
    }
  
  DBGLOG(DBG, "NewContext[" << ctx_id << "]::compute(): heads = " << *(heads->getHeads()));
  // send heads to Evaluator
  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), heads, timeout);
  
  return read_and_send_k1_k2(parent_qid, true, k1, k2, eval, md);
}



NewBeliefState*
NewContext::next_guess(NewBeliefState* current_guess, 
		       NewBeliefState* guessing_input)
{
  std::size_t n = BeliefStateOffset::instance()->NO_BLOCKS();
  std::size_t s = BeliefStateOffset::instance()->SIZE_BS();

  std::size_t first_bit_set = guessing_input->getFirst();
  assert (first_bit_set % (s+1) == 0);
  std::size_t bit = first_bit_set;

  do
    {
      bit = guessing_input->getNext(bit);
      
      if (bit % (s+1) != 0)
	{
	  if (current_guess->test(bit) == NewBeliefState::DMCS_FALSE)
	    {
	      // set this bit to 1
	      current_guess->set(bit, NewBeliefState::DMCS_TRUE);

	      // set all bits before, except epsilon bits, to 0
	      std::size_t previous_bit = first_bit_set;
	      previous_bit = guessing_input->getNext(previous_bit);
	      while (previous_bit < bit)
		{
		  if (previous_bit % (s+1) != 0)
		    {
		      current_guess->set(previous_bit, NewBeliefState::DMCS_FALSE);
		    }
		  previous_bit = guessing_input->getNext(previous_bit);
		}

	      // have a new guess
	      break;
	    }
	}
    }
  while (bit);

  if (!bit)
    {
      delete current_guess;
      current_guess = NULL;
    }

  return current_guess;
}


bool
NewContext::read_and_send_k1_k2(std::size_t parent_qid,
				bool normal_solve,
				std::size_t& k1,
				std::size_t& k2,
				EvaluatorPtr eval,
				NewConcurrentMessageDispatcherPtr md)
{
  DBGLOG(DBG, "NewContext::read_and_send_k1_k2(): k1 = " << k1 << ", k2 = " << k2);
  if (k1 == 0 && k2 == 0)
    {
      std::pair<std::size_t, std::size_t> p = read_and_send(parent_qid, normal_solve, eval, md);
      std::size_t models_sent = p.first;
      std::size_t conflict_models = p.second;
    }
  else
    {
      std::pair<std::size_t, std::size_t> p = read_and_send(parent_qid, normal_solve, eval, md);
      std::size_t models_sent = p.first;
      std::size_t conflict_models = p.second;
      std::size_t models_counter = eval->getModelsCounter();

      DBGLOG(DBG, "NewContext::read_and_send_k1_k2(): models_counter  = " << models_counter);
      DBGLOG(DBG, "NewContext::read_and_send_k1_k2(): conflict_models = " << conflict_models);
      DBGLOG(DBG, "NewContext::read_and_send_k1_k2(): models_sent     = " << models_sent);

      assert (conflict_models <= models_counter);
      models_counter -= conflict_models;

      DBGLOG(DBG, "NewContext::read_and_send_k1_k2(): non-conflict models_counter  = " << models_counter);
      
      if (models_sent == 0)
	// there was no model in the range [k1,k2]. models_counter are the number of models before k1.
	{
	  assert (models_counter < k1);
	  k2 -= models_counter;
	  k1 -= models_counter;
	}
      else if (models_sent < k2 - k1 + 1)
	// the number models returned by the Evaluator is in between k1,k2.
	{
	  k2 = k2 - models_sent - k1 + 1;
	  k1 = 1;
	}
      else
	// the Evaluator returned exactly models in range [k1,k2]
	{
	  assert (models_sent == k2 - k1 + 1);
	  return true;
	}
    }

  return false;
}  

  
// Read from EVAL_OUT_MQ[index] until getting (heads, NULL).
// Return the number of models received.
// Note that this function doesn't care about (k1, k2). The Evaluator and the outter loop take care of this issue.
std::pair<std::size_t, std::size_t>
NewContext::read_and_send(std::size_t parent_qid,
			  bool normal_solve,
			  EvaluatorPtr eval,
			  NewConcurrentMessageDispatcherPtr md)
{
  std::size_t models_sent = 0;
  std::size_t conflict_models = 0;

  int timeout = 0;
  while (1)
    {
      HeadsBeliefStatePair* res = md->receive<HeadsBeliefStatePair>(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, eval->getOutQueue(), timeout);
      assert (res);

      Heads* heads = res->first;
      NewBeliefState* belief_state = res->second;

      delete res;
      res = 0;
      
      if (belief_state == NULL)
	{
	  DBGLOG(DBG, "NewContext[" << ctx_id << "]::read_and_send(). Got res = NULL");
	  break;
	}
      else
	{
	  DBGLOG(DBG, "NewContext[" << ctx_id << "]::read_and_send(). Got res = " << *belief_state);
	  if (send_out_result(parent_qid, normal_solve, heads, belief_state, md))
	    {
	      ++models_sent;
	      DBGLOG(DBG, "NewContext[" << ctx_id << "]::read_and_send(). successfully sent.");
	    }
	  else
	    {
	      ++conflict_models;
	      DBGLOG(DBG, "NewContext[" << ctx_id << "]::read_and_send().NOT sent due to wrong guess.");
	    }
	}
    }

  return std::make_pair<std::size_t, std::size_t>(models_sent, conflict_models);
}


bool
NewContext::send_out_result(std::size_t parent_qid,
			    bool normal_solve,
			    Heads* heads,
			    NewBeliefState* belief_state,
			    NewConcurrentMessageDispatcherPtr md)
{
  if (heads->getHeads() != NULL)
    {
#if 0
      if (normal_solve)
	{
	  HeadsPlusBeliefState* heads_plus_bs = static_cast<HeadsPlusBeliefState*>(heads);
	  const NewBeliefState* input_bs = heads_plus_bs->getInputBeliefState();

	  // combine
	  DBGLOG(DBG, "NewContext::send_out_result(): belief_state = " << *belief_state);
	  DBGLOG(DBG, "NewContext::send_out_result(): input_bs     = " << *input_bs);
	  if (!belief_state->consistent_with(*input_bs, ctx_id,
					     BeliefStateOffset::instance()->getStartingOffsets()))
	    {
	      DBGLOG(DBG, "NewContext::send_out_result(): conflict detected!");
	      delete belief_state;
	      belief_state = NULL;

	      // delete heads_plus_bs?
	      // where to delete input_bs?

	      return false;
	    }
	    
	  (*belief_state) = (*belief_state) | (*input_bs); 
	  DBGLOG(DBG, "NewContext::send_out_result(): combined bs = " << *belief_state);
	}
      // otherwise, this context broke a cycle and we don't have input_bs
#endif

      // project
      std::size_t parent_ctx_id = invoker_from_qid(parent_qid);
      ReturnPlanMap::const_iterator it = return_plan->find(parent_ctx_id);
      assert (it != return_plan->end());
      NewBeliefState* interface = it->second;
      (*belief_state) = (*belief_state) & (*interface);
    }

  ReturnedBeliefState* rbs = new ReturnedBeliefState(belief_state, parent_qid);

  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);
  return true;
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
