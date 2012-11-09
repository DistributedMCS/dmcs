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
#include "mcs/ForwardMessage.h"
#include "dmcs/DLVEvaluator.h"

namespace dmcs {

// for leaf contexts
NewContext::NewContext(std::size_t cid,
		       InstantiatorPtr i,
		       BeliefTablePtr lsig,
		       ReturnPlanMapPtr return_plan)
  : is_leaf(true),
    ctx_id(cid),
    query_counter(0),
    inst(i),
    bridge_rules(BridgeRuleTablePtr()),
    local_signature(lsig),
    return_plan(return_plan),
    neighbors(NewNeighborVecPtr()),
    joiner(StreamingJoinerPtr())
{ }



// for intermediate contexts
NewContext::NewContext(std::size_t cid,
		       std::size_t pack_size,
		       InstantiatorPtr i,
		       BeliefTablePtr lsig,
		       ReturnPlanMapPtr return_plan,
		       BridgeRuleTablePtr br,
		       NewNeighborVecPtr nbs)
  : is_leaf(nbs->size() == 0),
    ctx_id(cid),
    query_counter(0),
    inst(i),
    bridge_rules(br),
    local_signature(lsig),
    return_plan(return_plan),
    neighbors(nbs),
    joiner(new StreamingJoiner(pack_size, nbs))
{ }




std::size_t
NewContext::getRequestOffset()
{
  return ctx_offset;
}



NewNeighborVecPtr
NewContext::getNeighbors()
{
  return neighbors;
}


void
NewContext::startup(NewConcurrentMessageDispatcherPtr md,
		    RequestDispatcherPtr rd,
		    NewJoinerDispatcherPtr jd)
{
  // Register REQUESTMQ to md
  ctx_offset = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::REQUEST_MQ);
  rd->registerIdOffset(ctx_id, ctx_offset);

  if (!is_leaf)
    {
      assert (joiner);
      joiner->registerJoinIn(ctx_offset, md);
    }

  // Start evaluator thread
  InstantiatorWPtr inst_wptr(inst);
  EvaluatorPtr eval = inst->createEvaluator(inst_wptr);
  inst->startThread(eval, ctx_id, local_signature, md);

  int timeout = 0;
  while (1)
    {
      DBGLOG(DBG, "NewContext[" << ctx_id << "]::startup(): Waiting at REQUEST_MQ[" << ctx_offset << "]");
      // Listen to the REQUEST_MQ
      ForwardMessage* fwd_mess = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_offset, timeout);

      DBGLOG(DBG, "NewContext[" << ctx_id << "]::startup(): Got message: " << *fwd_mess);
      
      std::size_t parent_qid = fwd_mess->qid;
      if (is_shutdown(parent_qid))
	{
	  break;
	}

      std::size_t k1 = fwd_mess->k1;
      std::size_t k2 = fwd_mess->k2;

      // Bad requests are not allowed
      assert ((k1 == 0 && k2 == 0) || (0 < k1 && k1 < k2+1));

      if (is_leaf)
	{
	  assert ( jd == NewJoinerDispatcherPtr() );
	  leaf_process_request(parent_qid, eval, md, k1, k2);
	}
      else
	{
	  const NewHistory& parent_history = fwd_mess->history;
	  NewHistory::const_iterator it = parent_history.find(ctx_id);
	  if (it == parent_history.end())
	    // no cycle detected
	    {
	      NewHistory history = parent_history;
	      history.insert(ctx_id);
	      intermediate_process_request(parent_qid, history, eval, md, jd, k1, k2);
	    }
	  //	  else
	    // cycle detected
	  //  {
	  //    break_cycle(parent_qid, eval, md, jd, k1, k2);
	  //  }
	}

      // Send the marker for the end of models
      ReturnedBeliefState* rbs = new ReturnedBeliefState(NULL, parent_qid);
      md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);      
    } 

  // send NULL to evaluator to tell it to stop
  Heads* end_heads = NULL;
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), end_heads, timeout);
  inst->stopThread(eval);
}


void
NewContext::leaf_process_request(std::size_t parent_qid,
				 EvaluatorPtr eval,
				 NewConcurrentMessageDispatcherPtr md,
				 std::size_t k1,
				 std::size_t k2)
{
  // send heads to Evaluator
  Heads* heads = new Heads(NULL, k1, k2);
  int timeout = 0;
  DBGLOG(DBG, "NewContext[" << ctx_id << "]::leaf_process_request: Send (" << k1 << ", " << k2  << ") to eval[" << eval->getInQueue() << "]");
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), heads, timeout);

  // read the output from EVAL_OUT_MQ
  read_and_send(parent_qid, true, eval, md);
}


void
NewContext::intermediate_process_request(std::size_t parent_qid,
					 const NewHistory& history,
					 EvaluatorPtr eval,
					 NewConcurrentMessageDispatcherPtr md,
					 NewJoinerDispatcherPtr jd,
					 std::size_t k1,
					 std::size_t k2)
{
  assert ((k1 == 0 && k2 == 0) || (0 < k1 && k1 < k2+1));

  while (1)
    {
      // prepare the heads
      std::size_t this_qid = query_id(ctx_id, ++query_counter);
      DBGLOG(DBG, "NewContext[" << ctx_id << "]::intermediate_process_request: trigger join with query_id = " << this_qid << " " << detailprint(this_qid));
      ReturnedBeliefState* rbs = joiner->trigger_join(this_qid, history, md, jd);
      if (rbs->belief_state == NULL)
	{
	  break;
	}

      NewBeliefState* input = rbs->belief_state;
      DBGLOG(DBG, "NewContext[" << ctx_id << "]::intermediate_process_request(): got input = " << *input);
      Heads* heads = evaluate_bridge_rules(bridge_rules, input, k1, k2,
					   BeliefStateOffset::instance()->getStartingOffsets());      

      DBGLOG(DBG, "NewContext[" << ctx_id << "]::intermediate_process_request(): heads = " << *(heads->getHeads()));
      // send heads to Evaluator
      int timeout = 0;
      md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), heads, timeout);
      
      if (read_and_send_k1_k2(parent_qid, true, k1, k2, eval, md))
	{
	  break;
	}
    }
}


#if 0
void
NewContext::break_cycle(std::size_t parent_qid,
			EvaluatorPtr eval,
			NewConcurrentMessageDispatcherPtr md,
			NewJoinerDispatcherPtr jd,
			std::size_t k1,
			std::size_t k2)
{
  DBGLOG(DBG, "NewContext[" << ctx_id << "]::break_cycle()");
  Heads* heads = collect_bridge_heads(bridge_rules, k1, k2);

  // send heads to Evaluator
  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), heads, timeout);

  read_and_send_k1_k2(parent_qid, false, k1, k2, eval, md);
}
#endif


bool
NewContext::read_and_send_k1_k2(std::size_t parent_qid,
				bool normal_solve,
				std::size_t k1,
				std::size_t k2,
				EvaluatorPtr eval,
				NewConcurrentMessageDispatcherPtr md)
{
  if (k1 == 0 && k2 == 0)
    {
      std::size_t models_sent = read_and_send(parent_qid, normal_solve, eval, md);
    }
  else
    {
      std::size_t models_sent = read_and_send(parent_qid, normal_solve, eval, md);
      std::size_t models_counter = eval->getModelsCounter();
      
      if (models_sent == 0)
	// there was no model in the range [k1,k2]. models_counter are the number of models before k1.
	{
	  DBGLOG(DBG, "DLVEvaluator::read_and_send_k1_k2(): models_counter = " << models_counter);
	  DBGLOG(DBG, "DLVEvaluator::read_and_send_k1_k2(): models_sent    = " << models_sent);
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
std::size_t
NewContext::read_and_send(std::size_t parent_qid,
			  bool normal_solve,
			  EvaluatorPtr eval,
			  NewConcurrentMessageDispatcherPtr md)
{
  std::size_t models_counter = 0;
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
	  ++models_counter;
	  send_out_result(parent_qid, normal_solve, heads, belief_state, md);
	}
    }

  return models_counter;
}


void
NewContext::send_out_result(std::size_t parent_qid,
			    bool normal_solve,
			    Heads* heads,
			    NewBeliefState* belief_state,
			    NewConcurrentMessageDispatcherPtr md)
{
  if (heads->getHeads() != NULL)
    {
      if (normal_solve)
	{
	  HeadsPlusBeliefState* heads_plus_bs = static_cast<HeadsPlusBeliefState*>(heads);
	  const NewBeliefState* input_bs = heads_plus_bs->getInputBeliefState();

	  // combine
	  (*belief_state) = (*belief_state) | (*input_bs); 
	}
      // otherwise, this context broke a cycle and we don't have input_bs

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
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
