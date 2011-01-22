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


#include "solver/RelSatSolver.h"

#include "dmcs/Log.h"

namespace dmcs {

RelSatSolver::RelSatSolver(bool il,
			   std::size_t my_id_,
			   const TheoryPtr& theory_, 
			   const SignaturePtr& sig_,
			   const BeliefStatePtr& localV_,
			   //			   const ProxySignatureByLocalPtr& mixed_sig_,
			   std::size_t system_size_,
			   MessagingGatewayBCPtr& mg_,
			   ConcurrentMessageQueuePtr& dsn,
			   ConcurrentMessageQueuePtr& srn)
  : is_leaf(il),
    my_id(my_id_),
    theory(theory_), 
    sig(sig_),
    localV(localV_),
    //    mixed_sig(mixed_sig_),
    system_size(system_size_),
    mg(mg_),
    dmcs_sat_notif(dsn),
    sat_router_notif(srn),
    xInstance(new SATInstance(std::cerr)),
    xSATSolver(new SATSolver(xInstance, std::cerr, this))
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
  BeliefSet& b = (*input)[it->ctxId - 1];
  int lid = it->localId;

  int ucl = testBeliefSet(b, it->origId) ? lid : -lid;

  DMCS_LOG_TRACE("input:    " << *input);
  DMCS_LOG_TRACE("context:  " << it->ctxId - 1);
  DMCS_LOG_TRACE("bset:     " << b);
  DMCS_LOG_TRACE("localid:  " << lid);

  DMCS_LOG_TRACE("Adding unit clause " << ucl << " to local theory.");

  xInstance->add_unit_clause(ucl);
}



// return false when there is no more input to read
bool
RelSatSolver::prepare_input()
{
  // read joined input from Joiner. We need to keep the input to build
  // complete model wrt the interface
  std::size_t prio = 0;
  int timeout      = 0;

  DMCS_LOG_TRACE("Staring at JOIN_OUT_MQ for the next input!");

  input = mg->recvModel(ConcurrentMessageQueueFactory::JOIN_OUT_MQ, prio, timeout);

  if (input == 0)
    {
      return false;
    }

  DMCS_LOG_TRACE("input received!");

  // then add input to the SATSolver's theory. We only add the atoms
  // that come from our neighbors' interface
  const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);

  // those are not in this range are my bridge atoms, which I need to
  // get truth value from the input
  SignatureByCtx::const_iterator low = local_sig.lower_bound(my_id);
  SignatureByCtx::const_iterator up  = local_sig.upper_bound(my_id);

  DMCS_LOG_TRACE(" Updating input from bridge signature...");

  for (SignatureByCtx::const_iterator it = local_sig.begin(); it != low; ++it)
    {
      update_bridge_input(it);
    }

  for (SignatureByCtx::const_iterator it = up; it != local_sig.end(); ++it)
    {
      update_bridge_input(it);
    }

  return true;
}



void
RelSatSolver::solve()
{
  // wait for conflict and partial_ass from Handler
  DMCS_LOG_TRACE(" Fresh solving. Wait for a message from DMCS");
  ConflictNotification* cn;
  void *ptr         = static_cast<void*>(&cn);
  unsigned int p    = 0;
  std::size_t recvd = 0;

  dmcs_sat_notif->receive(ptr, sizeof(cn), recvd, p);

  if (ptr && cn)
    {
      Conflict* conflict           = cn->conflict;
      BeliefState* new_partial_ass = cn->partial_ass;

      DMCS_LOG_TRACE(" Got a message from DMCS. conflict = " << *conflict << ". new_partial_ass = " << *new_partial_ass);

      /*
	if (partial_ass == 0)
	{
	partial_ass = new_partial_ass;
	DMCS_LOG_TRACE("First time. Going to start");
	}
	else if ((*partial_ass) != (*new_partial_ass))
	{ // now restart
	partial_ass = new_partial_ass;
	DMCS_LOG_TRACE("New partial_ass. Going to restart");
	}
	else
	{ // continue
	}*/
  
      // remove input part of the theory (from last solve)

      relsat_enum eResult;

      if (is_leaf)
	{
	  DMCS_LOG_TRACE("Leaf case. Solve now.");
	  eResult = xSATSolver->eSolve();
	}
      else
	{
	  DMCS_LOG_TRACE("Intermediate case.");

	  while (1)
	    {
	      DMCS_LOG_TRACE("Prepare input before solving.");
	      xInstance->removeLastInput();
	      if (!prepare_input())
		{
		  break;
		}
	      DMCS_LOG_TRACE("A fresh solving.");
	      xSATSolver->refresh();
	      eResult = xSATSolver->eSolve();
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
RelSatSolver::receiveUNSAT()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  // send a NULL pointer to the SatOutputMessageQueue
  //mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);

  // 
}



void
RelSatSolver::receiveSolution(DomainValue* _aAssignment, int _iVariableCount)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  BeliefState* bs;
  // copy input
  if (is_leaf)
    {
      bs = new BeliefState(system_size, BeliefSet());
    }
  else
    {
      bs = new BeliefState(*input);
      DMCS_LOG_TRACE("input: " << *input);
    }

  DMCS_LOG_TRACE("bs:    " << *bs);

  // set epsilon bit of my position so that the invoker knows this is SATISFIABLE
  BeliefSet& belief = (*bs)[my_id-1];
  setEpsilon(belief);                

  ///@todo: MD: just need to look at real local atoms
  for (int i = 0; i < _iVariableCount; i++) 
    {
      assert(_aAssignment[i] != NON_VALUE);

      if (_aAssignment[i]) 
	{
	  const SignatureByLocal& local_sig = boost::get<Tag::Local>(*sig);
	  dmcs::SignatureByLocal::const_iterator loc_it = local_sig.find(i+1);
	  
	  // it must show up in the signature
	  assert (loc_it != local_sig.end());
	  
	  std::size_t cid = loc_it->ctxId - 1;
	  
	  // just to be safe
	  assert (cid < system_size);
	  
	  BeliefSet& belief = (*bs)[cid];

	  belief.set(loc_it->origId);
	}
    }

  DMCS_LOG_TRACE("After adding result: bs = " << *bs);

  // now put this BeliefState to the SatOutputMessageQueue
  mg->sendModel(bs, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);

  DMCS_LOG_TRACE("Solution sent: " << *bs);
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
