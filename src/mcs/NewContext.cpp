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

#include "boost/thread.hpp"

namespace dmcs {

NewContext::NewContext(std::size_t cid,
		       InstantiatorPtr i,
		       BeliefTablePtr ex_sig)
  : is_leaf(true),
    ctx_id(cid),
    query_counter(0),
    answer_counter(0),
    inst(i),
    export_signature(ex_sig),
    bridge_rules(BridgeRuleTablePtr()),
    neighbors(NewNeighborVecPtr()),
    offset2index(NeighborOffset2IndexPtr()),
    joiner(StreamingJoinerPtr())
{ }

NewContext::NewContext(std::size_t cid,
		       InstantiatorPtr i,
		       BeliefTablePtr ex_sig,
		       BridgeRuleTablePtr br,
		       NewNeighborVecPtr nbs,
		       NeighborOffset2IndexPtr o2i)
  : is_leaf(nbs->size() == 0),
    ctx_id(cid),
    query_counter(0),
    answer_counter(0),
    inst(i),
    export_signature(ex_sig),
    bridge_rules(br),
    neighbors(nbs),
    offset2index(o2i),
    joiner(new StreamingJoiner(cid, nbs, o2i))
{ }


void
NewContext::operator()(NewConcurrentMessageDispatcherPtr md,
		       NewJoinerDispatcherPtr jd)
{
  // register REQUEST_MQ to md
  ConcurrentMessageQueuePtr my_request_mq(new ConcurrentMessageQueue(md->getQueueSize()));
  md->registerMQ(my_request_mq, NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id);

  // start evaluator thread
  InstantiatorWPtr inst_wptr(inst);
  EvaluatorPtr eval = inst->createEvaluator(inst_wptr);
  inst->startThread(eval, ctx_id, export_signature, md);

  int timeout = 0;
  while (1)
    {
      // listen to the REQUEST_MQ

      ForwardMessage* fwd_mess = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id, timeout);
      
      std::size_t parent_qid = fwd_mess->query_id;
      if (shutdown(parent_qid))
	{
	  break;
	}

      std::size_t k1 = fwd_mess->k1;
      std::size_t k2 = fwd_mess->k2;

      while (1)
	{
	  Heads* heads = NULL;
	  
	  // intermediate context
	  if (!is_leaf)
	    {
	      std::size_t this_qid = query_id(ctx_id, query_counter++);
	      ReturnedBeliefState* rbs = joiner->trigger_join(this_qid, k1, k2, md, jd);
	      if (rbs->belief_state == NULL)
		{
		  rbs->query_id = parent_qid;		
		  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);
		  break;
		}

	      NewBeliefState* input = rbs->belief_state;
	      heads = evaluate_bridge_rules(bridge_rules, input, BeliefStateOffset::instance()->getStartingOffsets());
	    }	  

	  // send heads to Evaluator
	  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), heads, timeout);

	  if (k1 == 0 && k2 == 0)
	    {
	      read_all(parent_qid, eval, md);
	      if (is_leaf)
		{
		  ReturnedBeliefState* rbs = new ReturnedBeliefState(NULL, parent_qid);
		  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);
		  break;
		}
	    }
	  else
	    {
	      if (read_until_k2(k1, k2, parent_qid, eval, md))
		{
		  std::cerr << "Got up to k2 = " << k2 << ". Now break!" << std::endl;
		  break;
		}
	    }
	} // end while solving up to k2
    } // end while listening to request

  // send NULL to evaluator
  Heads* end_heads = new Heads(NULL);
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, eval->getInQueue(), end_heads, timeout);
  inst->stopThread(eval);
}


void
NewContext::read_all(std::size_t parent_qid,
		     EvaluatorPtr eval,
		     NewConcurrentMessageDispatcherPtr md)
{
  while (1)
    {
      int timeout = 0;
      HeadsBeliefStatePair* res = md->receive<HeadsBeliefStatePair>(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, eval->getOutQueue(), timeout);
      
      Heads* heads = res->first;
      NewBeliefState* belief_state = res->second;
      delete res;
      res = 0;
      
      if (belief_state == NULL)
	{
	  break;
	}
      else
	{
	  send_out_result(parent_qid, heads, belief_state, md);
	}
    }
}


bool
NewContext::read_until_k2(std::size_t k1,
			  std::size_t k2,
			  std::size_t parent_qid,
			  EvaluatorPtr eval,
			  NewConcurrentMessageDispatcherPtr md)
{
  assert (k1 > 0 && k2 > k1);

  /*
  if (answer_counter >= k1)
    {
      reset();
      }*/

  while (answer_counter < k1-1)
    {
      int timeout = 0;
      HeadsBeliefStatePair* res = md->receive<HeadsBeliefStatePair>(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, eval->getOutQueue(), timeout);

      NewBeliefState* belief_state = res->second;
      delete res;
      res = 0;

      if (belief_state == NULL)
	{
	  return false;
	}
      else
	{
	  answer_counter++;
	  delete belief_state;
	  belief_state = 0;
	}
    }

  while (answer_counter < k2)
    {
      int timeout = 0;
      HeadsBeliefStatePair* res = md->receive<HeadsBeliefStatePair>(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, eval->getOutQueue(), timeout);

      Heads* heads = res->first;
      NewBeliefState* belief_state = res->second;

      delete res;
      res = 0;

      if (belief_state == NULL)
	{
	  return false;
	}
      else
	{
	  send_out_result(parent_qid, heads, belief_state, md);
	  answer_counter++;
	}
    }

  std::cerr << "answer_counter = " << answer_counter << std::endl;
  if (answer_counter == k2)
    {
      return true;
    }
  
  return false;
}


void
NewContext::send_out_result(std::size_t parent_qid,
			    Heads* heads,
			    NewBeliefState* belief_state,
			    NewConcurrentMessageDispatcherPtr md)
{
  if (heads != NULL)
    {
      HeadsPlusBeliefState* heads_plus_bs = static_cast<HeadsPlusBeliefState*>(heads);
      const NewBeliefState* input_bs = heads_plus_bs->getInputBeliefState();
      
      // combine
      (*belief_state) = (*belief_state) | (*input_bs); 
    }

  ReturnedBeliefState* rbs = new ReturnedBeliefState(belief_state, parent_qid);

  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs, timeout);
}


void
NewContext::reset()
{
  answer_counter = 0;
  // reset solver
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
