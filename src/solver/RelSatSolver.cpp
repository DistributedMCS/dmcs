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

#include <boost/thread.hpp>

#include "dmcs/BeliefCombination.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "solver/RelSatSolver.h"
#include "relsat-20070104/RelSatHelper.h"
#include "relsat-20070104/SATInstance.h"

#include "dmcs/Log.h"

namespace dmcs {

RelSatSolver::RelSatSolver(bool il,
			   std::size_t my_id_,
			   std::size_t my_sid,
			   const TheoryPtr& theory_, 
			   const SignaturePtr& sig_,
			   const BeliefStatePtr& localV_,
			   const HashedBiMap* co,
			   std::size_t system_size_,
			   MessagingGatewayBC* mg_,
			   ConcurrentMessageQueue* dsn,
			   std::size_t p)
  : is_leaf(il),
    my_id(my_id_),
    my_session_id(my_sid),
    theory(theory_), 
    sig(sig_),
    localV(localV_),
    c2o(co),
    system_size(system_size_),
    mg(mg_),
    dmcs_sat_notif(dsn),
    xInstance(new SATInstance(std::cerr)),
    xSATSolver(new SATSolver(xInstance, std::cerr, this)),
    port(p),
    input(0)
{
  xInstance->readTheory(theory, sig->size());
}


RelSatSolver::~RelSatSolver()
{
  delete xInstance;
  delete xSATSolver;
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

  DMCS_LOG_TRACE(port << ": Waiting at JOIN_OUT_MQ for the next input!");

  struct MessagingGatewayBC::ModelSession ms = 
    mg->recvModel(ConcurrentMessageQueueFactory::JOIN_OUT_MQ, prio, timeout);

  // old input is now outdated
  if (input)
    {
      DMCS_LOG_TRACE(port << ": Going to delete old input");
      delete input;
      input = 0;
    }

  input = ms.m;
  std::size_t sid = ms.sid;

  if (input == 0)
    {
      return false;
    }

  if (sid != my_session_id)
    {
      assert (sid < my_session_id);
      DMCS_LOG_TRACE(port << ": Receive old joined input from sid = " << sid << ", while my current session id = " << my_session_id);
      return false;
    }

  DMCS_LOG_TRACE(port << ": input received!");

  xInstance->setSizeWPartialAss(xInstance->iClauseCount());
  
  // then add input to the SATSolver's theory. We only add the atoms
  // that come from our neighbors' interface
  const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);

  // those are not in this range are my bridge atoms, which I need to
  // get truth value from the input
  SignatureByCtx::const_iterator low = local_sig.lower_bound(my_id);
  SignatureByCtx::const_iterator up  = local_sig.upper_bound(my_id);

  //DMCS_LOG_TRACE(port << ":  Updating input from bridge signature...");

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
RelSatSolver::solve()
{
  // wait for conflict and partial_ass from DMCS
  DMCS_LOG_TRACE(port << ":  Fresh solving. Wait for a message from DMCS");
  ConflictNotification* cn;
  void *ptr         = static_cast<void*>(&cn);
  unsigned int p    = 0;
  std::size_t recvd = 0;

  dmcs_sat_notif->receive(ptr, sizeof(cn), recvd, p);

  if (ptr && cn)
    {
      parent_session_id = cn->session_id;

      //delete cn;
      //cn = 0;

      // remove input part of the theory (from last solve)

      relsat_enum eResult;

      if (is_leaf)
	{
	  DMCS_LOG_TRACE(port << ": Leaf case. Solve now.");
	  eResult = xSATSolver->eSolve();
	  xSATSolver->refresh();
	}
      else
	{
	  DMCS_LOG_TRACE(port << ": Intermediate case.");

	  while (1)
	    {
	      DMCS_LOG_TRACE(port << ": Prepare input before solving.");
	      DMCS_LOG_TRACE(port << ": xIntance.size() before removing input = " << xInstance->iClauseCount());
	      xInstance->removeInput();
	      DMCS_LOG_TRACE(port << ": xIntance.size() after removing input = " << xInstance->iClauseCount());
	      if (!prepare_input())
		{
		  DMCS_LOG_TRACE(port << ": Got NULL input from JOIN_OUT_MQ. Send a NULL model to OUT_MQ to inform OutputThread");
		  mg->sendModel(0, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0); 
		  break;
		}
	      DMCS_LOG_TRACE(port << ": A fresh solving. input = " << *input);
	      
	      boost::this_thread::interruption_point();

	      eResult = xSATSolver->eSolve();
	      xSATSolver->refresh();
	    }
	}

      ///@todo what todo with eResult?
    }
  else
    {
      DMCS_LOG_FATAL("Got null message: " << ptr << " " << cn);
      assert(ptr != 0 && cn != 0);
    }
}



void
RelSatSolver::receiveEOF()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
}



void
RelSatSolver::receiveUNSAT(ClauseList& learned_clauses)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (is_leaf)
    {
      DMCS_LOG_TRACE(port << ": UNSAT. Send a NULL pointer to OUT_MQ to close this session.");
      mg->sendModel(0, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
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

  DMCS_LOG_TRACE(port << ": MODEL from SAT: bs = " << *bs);

  // project to my output interface
  DMCS_LOG_TRACE(port << ": localV  = " << *localV);
  project_to(bs, localV);

  // attach parent_session_id to the output models

  DMCS_LOG_TRACE(port << ": Going to send: " << *bs << ", with parent session id = " << parent_session_id);

  // now put this PartialBeliefState to the SatOutputMessageQueue
  mg->sendModel(bs, parent_session_id, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);

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
