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
 * @file   RelSatSolver.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  4 17:27:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/BeliefCombination.h"
#include "dmcs/AskNextNotification.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "solver/RelSatSolver.h"
#include "relsat-20070104/RelSatHelper.h"
#include "relsat-20070104/SATInstance.h"

#include "dmcs/Log.h"

#include <boost/thread.hpp>



namespace dmcs {

RelSatSolver::RelSatSolver(bool il,
			   std::size_t mid,
			   std::size_t msid,
			   const TheoryPtr& t, 
			   const SignaturePtr& ls,
			   const HashedBiMap* co,
			   std::size_t ss,
			   QueryPlan* qp,
			   ConcurrentMessageQueue* jsn,
			   MessagingGatewayBC* m)
  : is_leaf(il),
    my_id(mid),
    my_session_id(msid),
    theory(t), 
    sig(ls),
    c2o(co),
    system_size(ss),
    query_plan(qp),
    joiner_sat_notif(jsn),
    mg(m),
    xInstance(new SATInstance(std::cerr)),
    xSATSolver(new SATSolver(xInstance, std::cerr, this)),
    input(0),
    input_buffer(new PartialBeliefStateBuf(CIRCULAR_BUF_SIZE))
{
  xInstance->readTheory(theory, sig->size());
}



RelSatSolver::~RelSatSolver()
{
  delete xInstance;
  delete xSATSolver;

  for (PartialBeliefStateBuf::const_iterator it = input_buffer->begin(); it != input_buffer->end(); ++it)
    {
      delete *it;
    }
}



int
RelSatSolver::solve(const TheoryPtr& /* theory */, std::size_t /* sig_size */)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  return 42; ///@todo  nothing here??
}



void
RelSatSolver::update_bridge_input(SignatureByCtx::const_iterator it)
{
  PartialBeliefSet& b = (*input)[it->ctxId - 1];
  
  int lid = it->localId;

  PartialBeliefSet::TruthVal val = testBeliefSet(b, it->origId);

  assert (val != PartialBeliefSet::DMCS_UNDEF);

  int ucl = (val == PartialBeliefSet::DMCS_TRUE) ? lid : -lid;

#if 0
  DMCS_LOG_TRACE("input:    " << *input);
  DMCS_LOG_TRACE("context:  " << it->ctxId - 1);
  DMCS_LOG_TRACE("bset:     " << b);
  DMCS_LOG_TRACE("localid:  " << lid);

  DMCS_LOG_TRACE("Adding unit clause " << ucl << " to local theory.");
#endif // 0

  xInstance->add_unit_clause(ucl);
}



void
RelSatSolver::update_local_input(PartialBeliefSet& my_belief_set, 
				 SignatureByCtx::const_iterator low, 
				 SignatureByCtx::const_iterator up)
{
  for (SignatureByCtx::const_iterator it = low; it != up; ++it)
    {
      assert (it->localId == it->origId);

      int oid = it->origId;

      PartialBeliefSet::TruthVal val = testBeliefSet(my_belief_set, oid);

      if (val != PartialBeliefSet::DMCS_UNDEF)
	{
	  int ucl = (val == PartialBeliefSet::DMCS_TRUE) ? oid : -oid;
	  xInstance->add_unit_clause(ucl);
	}
    }
}



// return false when there is no more input to read
bool
RelSatSolver::prepare_input()
{
  // read joined input from Joiner. We need to keep the input to build
  // complete model wrt the interface
  std::size_t prio = 0;
  int timeout      = 0;

  DMCS_LOG_TRACE("Waiting at JOIN_OUT_MQ for the next input!");

  struct MessagingGatewayBC::ModelSession ms = 
    mg->recvModel(ConcurrentMessageQueueFactory::JOIN_OUT_MQ, prio, timeout);

  input = ms.m;
  std::size_t sid = ms.sid;

  if (input == 0)
    {
      return false;
    }

  if (sid != my_session_id)
    {
      assert (sid < my_session_id);
      DMCS_LOG_TRACE("Receive old joined input from sid = " << sid << ", while my current session id = " << my_session_id);
      return false;
    }

  DMCS_LOG_TRACE("input received!");

  xInstance->setSizeWPartialAss(xInstance->iClauseCount());
  
  // then add input to the SATSolver's theory. We only add the atoms
  // that come from our neighbors' interface
  const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);

  // those are not in this range are my bridge atoms, which I need to
  // get truth value from the input
  SignatureByCtx::const_iterator low = local_sig.lower_bound(my_id);
  SignatureByCtx::const_iterator up  = local_sig.upper_bound(my_id);

  //DMCS_LOG_TRACE(" Updating input from bridge signature...");

  for (SignatureByCtx::const_iterator it = local_sig.begin(); it != low; ++it)
    {
      update_bridge_input(it);
    }

  for (SignatureByCtx::const_iterator it = up; it != local_sig.end(); ++it)
    {
      update_bridge_input(it);
    }

  PartialBeliefSet& my_belief_set = (*input)[my_id-1];
  if (isEpsilon(my_belief_set))
    {
      update_local_input(my_belief_set, low, up);
    }

  return true;
}



void
RelSatSolver::refresh()
{
  //xInstance->removePartialAss();
  xSATSolver->refresh();
}



void
RelSatSolver::solve(std::size_t iv, std::size_t pa, std::size_t session_id, std::size_t k1, std::size_t k2)
{
  invoker = iv;
  parent_session_id = session_id;

  relsat_enum eResult;
  std::size_t models_sofar = 0;
  path = pa;

  if (is_leaf)
    {
      DMCS_LOG_TRACE("Leaf case. Solve now. k2 = " << k2);
      eResult = xSATSolver->eSolve((long int)k2, models_sofar);
      xSATSolver->refresh();
    }
  else
    {
      DMCS_LOG_TRACE("Intermediate case.");

      std::size_t left_to_request = k2;
      
      while (1)
	{
	  if (k2 > 0)
	    {
	      DMCS_LOG_TRACE("left_to_request = " << left_to_request);
	      DMCS_LOG_TRACE("models_sofar = " << models_sofar);
	      
	      assert (left_to_request >= models_sofar);
	      
	      left_to_request -= models_sofar;
	    }

	  if (k2 > 0 && left_to_request == 0)
	    {
	      DMCS_LOG_TRACE("Reached " << k2 << " models. Tell Joiner to shut up and get out. Also send OutputThread a NULL pointer.");

	      mg->sendModel(0, 0, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0); 
	      AskNextNotification* notif = new AskNextNotification(BaseNotification::SHUTUP, path, 0, 0, 0);
	      mg->sendNotification(notif, 0, ConcurrentMessageQueueFactory::SAT_JOINER_MQ, 0);
	      break;
	    }

	  DMCS_LOG_TRACE("Request another input from Joiner");
	  AskNextNotification* notif = new AskNextNotification(BaseNotification::NEXT, path, session_id, k1, k2);
	  mg->sendNotification(notif, 0, ConcurrentMessageQueueFactory::SAT_JOINER_MQ, 0);

	  DMCS_LOG_TRACE("Prepare input before solving.");
	  //DMCS_LOG_TRACE("xIntance.size() before removing input = " << xInstance->iClauseCount());
	  xInstance->removeInput();
	  //DMCS_LOG_TRACE("xIntance.size() after removing input = " << xInstance->iClauseCount());
	  if (!prepare_input())
	    {
	      DMCS_LOG_TRACE("Got NULL input from JOIN_OUT_MQ. Bailing out...");
	      mg->sendModel(0, 0, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);
	      break;
	    }
	  DMCS_LOG_TRACE("A fresh solving. input = " << *input);
	  
	  eResult = xSATSolver->eSolve((long int)left_to_request, models_sofar);
	  DMCS_LOG_TRACE("One limited solve finished. Number of solutions = " << models_sofar);
	  xSATSolver->refresh();
	  
	  bool was_cached;
	  store(input, input_buffer, true, was_cached);
	  DMCS_LOG_TRACE("One SOLVE finished.");
	}
    }
  
  ///@todo what todo with eResult?
}



void
RelSatSolver::receiveEOF()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  if (is_leaf)
    {
      DMCS_LOG_TRACE("EOF. Leaf case --> Send a NULL pointer to OUT_MQ to close this session.");
      mg->sendModel(0, path, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0); 
    }
}



void
RelSatSolver::receiveUNSAT(ClauseList& learned_clauses)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (is_leaf)
    {
      DMCS_LOG_TRACE("UNSAT. Send a NULL pointer to OUT_MQ to close this session.");
      mg->sendModel(0, path, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0); 
    }
}



void
RelSatSolver::receiveSolution(DomainValue* _aAssignment, int _iVariableCount)
{
  PartialBeliefState* bs;
  // copy input
  if (is_leaf)
    {
      bs = new PartialBeliefState(system_size, PartialBeliefSet());
    }
  else
    {
      bs = new PartialBeliefState(*input);
    }

  // set epsilon bit of my position so that the invoker knows this is SATISFIABLE
  PartialBeliefSet& belief = (*bs)[my_id-1];
  setEpsilon(belief);                

  ///@todo: MD: just need to look at real local atoms
  for (int i = 0; i < _iVariableCount; i++) 
    {
      assert(_aAssignment[i] != NON_VALUE);

      const SignatureByLocal& local_sig = boost::get<Tag::Local>(*sig);
      dmcs::SignatureByLocal::const_iterator loc_it = local_sig.find(i+1);
      
      // it must show up in the signature
      assert (loc_it != local_sig.end());

      std::size_t cid = loc_it->ctxId - 1;

      // just to be safe
      assert (cid < system_size);
      
      PartialBeliefSet& neighbor_belief = (*bs)[cid];
      setEpsilon(neighbor_belief);

      if (_aAssignment[i]) 
	{
	  setBeliefSet(neighbor_belief, loc_it->origId);
	}
      else
	{
	  setBeliefSet(neighbor_belief, loc_it->origId, PartialBeliefSet::DMCS_FALSE);
	}
    }

  //DMCS_LOG_TRACE("MODEL from SAT: bs = " << *bs);

  // project to my output interface
  BeliefStatePtr localV;
  if (invoker == 0)
    {
      localV = query_plan->getGlobalV();
    }
  else
    {
      localV = query_plan->getInterface(invoker, my_id);
    }

  project_to(bs, localV);

  DMCS_LOG_TRACE("Going to send: " << *bs << ", with path = " << path << ", parent session id = " << parent_session_id);
  // now put this PartialBeliefState to the SatOutputMessageQueue
  mg->sendModel(bs, path, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);
  // Models should be cleaned by OutputThread
}



void
RelSatSolver::print_local_theory()
{
  for (int i = 0; i < xInstance->iClauseCount(); ++i)
    {
      ::Clause* c = xInstance->pClause(i);
      DMCS_LOG_DEBUG(*c);
    }
}



} // namespace dmcs

// Local Variables:
// mode: C++
// End:
