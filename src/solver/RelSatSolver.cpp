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
			   const TheoryPtr& theory_, 
			   const SignaturePtr& sig_,
			   const BeliefStatePtr& localV_,
			   const VecSizeTPtr& oss,
			   //			   const ProxySignatureByLocalPtr& mixed_sig_,
			   const HashedBiMap* co,
			   std::size_t system_size_,
			   MessagingGatewayBC* mg_,
			   ConcurrentMessageQueue* dsn,
			   ConcurrentMessageQueue* srn,
			   std::size_t p)
  : is_leaf(il),
    my_id(my_id_),
    theory(theory_), 
    sig(sig_),
    localV(localV_),
    orig_sigs_size(oss),
    c2o(co),
    //    mixed_sig(mixed_sig_),
    system_size(system_size_),
    mg(mg_),
    dmcs_sat_notif(dsn),
    sat_router_notif(srn),
    learned_conflicts(new ConflictBufVec),
    new_conflicts_beg(new ConflictBufIterVec),
    trail(new Trail),
    xInstance(new SATInstance(std::cerr)),
    xSATSolver(new SATSolver(xInstance, std::cerr, this)),
    port(p),
    parent_conflicts(0),
    parent_ass(0),
    parent_decision(0)
{
  xInstance->readTheory(theory, sig->size());

  // prepare storage for conflicts learned by the solver
  std::size_t no_nbs = c2o->size();
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      ConflictBufPtr cb(new ConflictBuf(CIRCULAR_BUF_SIZE));
      learned_conflicts->push_back(cb);
      new_conflicts_beg->push_back(cb->begin());
    }
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



// return false when there is no more input to read
bool
RelSatSolver::prepare_input()
{
  // read joined input from Joiner. We need to keep the input to build
  // complete model wrt the interface
  std::size_t prio = 0;
  int timeout      = 0;

  DMCS_LOG_TRACE(port << ": Waiting at JOIN_OUT_MQ for the next input!");

  input = mg->recvModel(ConcurrentMessageQueueFactory::JOIN_OUT_MQ, prio, timeout);

  if (input == 0)
    {
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

  return true;
}



void
RelSatSolver::import_conflicts(const ConflictVec* conflicts)
{
  xInstance->setOrigTheorySize(xInstance->iClauseCount());

  std::size_t sig_size = sig->size();
  VariableSet xPositiveVariables(sig_size);
  VariableSet xNegativeVariables(sig_size);

  for (ConflictVec::const_iterator it = conflicts->begin(); it != conflicts->end(); ++it)
    {
      xPositiveVariables.vClear();
      xNegativeVariables.vClear();

      Conflict* conflict = *it;
      PartialBeliefSet& local_conflict = (*conflict)[my_id - 1];

      std::size_t bit = local_conflict.state_bit.get_first();
      VariableID eVar;

      do
	{
	  eVar = (VariableID)bit;

	  // a SAT solver should be intelligent enough NOT to send me
	  // a tautology, hence we don't check for tautology here
	  if (local_conflict.value_bit.test(bit))
	    {
	      xPositiveVariables.vAddVariable(eVar-1);
	    }
	  else
	    {
	      // actually it is 0 - (eVar+1), but we already had that
	      // eVar > 0, hence we can use this shortcut
	      xNegativeVariables.vAddVariable(eVar-1);
	    }
	  bit = local_conflict.state_bit.get_next(bit);
	}
      while (bit);

      assert(xNegativeVariables.iCount() + xPositiveVariables.iCount() > 0);
      ::Clause* pNewConstraint = new ::Clause((VariableList&)xPositiveVariables, 
					      (VariableList&)xNegativeVariables,
					      1);
      pNewConstraint->vSortVariableList();

      xInstance->vAddClause(pNewConstraint);
    }
}



void
RelSatSolver::import_partial_ass(const PartialBeliefState* partial_ass)
{
  xInstance->setSizeWConflict(xInstance->iClauseCount());
  for (Signature::const_iterator it = sig->begin(); it != sig->end(); ++it)
    {
      const PartialBeliefSet& b = (*partial_ass)[it->ctxId - 1];
      int lid = it->localId;

      PartialBeliefSet::TruthVal val = testBeliefSet(b, it->origId);
      
      if (val != PartialBeliefSet::DMCS_UNDEF)
	{
	  int ucl = (val == PartialBeliefSet::DMCS_TRUE) ? lid : -lid;
	  xInstance->add_unit_clause(ucl);
	}
    }
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
      if (parent_conflicts != 0)
	{
	  delete parent_conflicts;
	  parent_conflicts = 0;
	}

      if (parent_ass != 0)
	{
	  delete parent_ass;
	  parent_ass = 0;
	}

      if (parent_decision != 0)
	{
	  delete parent_decision;
	  parent_decision = 0;
	}

      parent_conflicts = cn->conflicts;
      parent_ass = cn->partial_ass;
      parent_decision = cn->decision;

      import_conflicts(parent_conflicts);

      DMCS_LOG_TRACE(port << ":  Got a message from DMCS. parent_conflicts = " << *parent_conflicts 
		     << ". parent_ass = " << *parent_ass
		     << ". parent_decision = " << *parent_decision);

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
	      xInstance->removeInput();
	      if (!prepare_input())
		{
		  DMCS_LOG_TRACE(port << ": Got NULL input from JOIN_OUT_MQ");
		  // send a NULL model to OUT_MQ to inform OutputThread
		  mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);
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
RelSatSolver::collect_learned_clauses(ClauseList& sat_learned_clauses)
{
  // in order not to be specific to relsat, we convert the learned
  // clauses to PartialBeliefState format for transferring over the
  // network, and at the neighbor, convert it back to ClauseList
  // format, with orig_id of the atoms.

  // a faster way to do this is directly convert the learned clauses
  // here, from our local_id to the orig_id in the atoms' original
  // context, having the result in ClauseList format. And the neighbor
  // can use these learned clauses directly without any
  // conversion. But then we have to transfer relsat's ClauseList and
  // hence it's not flexible to extend the implementation to make use
  // of other SAT solvers.

  assert (learned_conflicts->size() == new_conflicts_beg->size());

  ConflictBufVec::const_iterator lc_it = learned_conflicts->begin();
  ConflictBufIterVec::iterator beg_it = new_conflicts_beg->begin();

  for (; lc_it != learned_conflicts->end(); ++lc_it, ++beg_it)
    {
      ConflictBufPtr lc = *lc_it;
      ConflictBuf::iterator old_end_it = lc->end();
      if (lc->size() > 0)
	{
	  *beg_it = --old_end_it;
	}
      else
	{
	  *beg_it = old_end_it;
	}
    }

  for (int i = 0; i < sat_learned_clauses.iClauseCount(); ++i)
    {
      ::Clause* c = sat_learned_clauses.pClause(i);
      Conflict* conflict = new Conflict(system_size, PartialBeliefSet());
      PartialBeliefSet* neighbor_ref = 0;

      std::size_t from_neighbor = 0;

      for (int j = 0; j < c->iVariableCount(); ++j)
	{
	  const int atom = back_2_lit(c->eConstrainedLiteral(j));

	  assert (atom != 0);
	  const std::size_t abs_atom = std::abs(atom);

	  const SignatureByLocal& sig_by_local    = boost::get<Tag::Local>(*sig);
	  SignatureByLocal::const_iterator loc_it = sig_by_local.find(abs_atom);

	  assert (loc_it != sig_by_local.end());

	  const std::size_t orig_ctx = loc_it->ctxId;

	  if (orig_ctx == my_id || from_neighbor != orig_ctx) // let's get outta here
	    {
	      // orig_ctx == my_id: We don't care about conflict in the local context
	      //
	      // orig_ctx != from_neighbor: this learned clause
	      // includes atoms from different neighbors, we don't
	      // collect it

	      from_neighbor = 0;
	      break;
	    }
	  else if (from_neighbor == 0)
	    {
	      from_neighbor = orig_ctx;
	      neighbor_ref  = &((*conflict)[from_neighbor - 1]);
	    }
	  else
	    {
	      assert(false);
	    }

	  assert(neighbor_ref != 0);

	  if (atom > 0)
	    {
	      setBeliefSet(*neighbor_ref, abs_atom);
	    }
	  else
	    {
	      setBeliefSet(*neighbor_ref, abs_atom, PartialBeliefSet::DMCS_FALSE);
	    }
	}
      
      if (from_neighbor == 0)
	{
	  // we don't collect this clause
	  delete conflict;
	  conflict = 0;
	}
      else
	{
	  //setEpsilon(neighbor_ref);

	  // store this conflict at the right offset of the neighbor's id
	  const HashedBiMapByFirst& neighbor_context = boost::get<Tag::First>(*c2o);
	  HashedBiMapByFirst::const_iterator pair    = neighbor_context.find(from_neighbor);

	  assert (pair != neighbor_context.end());

	  const std::size_t neighbor_offset = pair->second;

	  ConflictBufPtr& storage_ref = (*learned_conflicts)[neighbor_offset];

	  if (!cached(conflict, storage_ref))
	    {
	      storage_ref->push_back(conflict);
	    }
	}
    } // for (int i = 0;



  lc_it = learned_conflicts->begin();
  beg_it = new_conflicts_beg->begin();

  for (; lc_it != learned_conflicts->end(); ++lc_it, ++beg_it)
    {
      ConflictBufPtr lc = *lc_it;
      ConflictBuf::iterator old_end_it = lc->end();
      if (*beg_it != lc->end())
	{
	  (*beg_it)++;
	}
    }
}



void
RelSatSolver::receiveEOF()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  // go to stack, pick the next flipping possibility and go. If the
  // stack is empty or there is no more possibility to flip then send
  // NULL to OUT_MQ

  if (trail->empty())
    {
      DMCS_LOG_TRACE(port << ": Empty stack, EOF. Send a NULL pointer to OUT_MQ");
      mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
    }
  else
    {
      DMCS_LOG_TRACE(port << ": EOF, now backtrack");

      ConflictNotification* next_flip;
      do 
	{
	  ChoicePointPtr& cp = trail->top();
	  next_flip = getNextFlip(cp, orig_sigs_size);
	  if (next_flip != 0)
	    {
	      break;
	    }
	}
      while (!trail->empty());

      if (next_flip == 0)
	{
	  DMCS_LOG_TRACE(port << ": Out of every thing. Send a NULL pointer to OUT_MQ");
	  mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
	}
      else
	{
	  DMCS_LOG_TRACE(port << ": Next possibility to push, let's notify router. next_flip == " << *next_flip);

	  ConflictNotification* ow_sat = 
	    (ConflictNotification*) overwrite_send(sat_router_notif, &next_flip, sizeof(next_flip), 0);
	  
	  if (ow_sat)
	    {
	      delete ow_sat;
	      ow_sat = 0;
	    }
	}
    }
}


void
RelSatSolver::receiveUNSAT()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (is_leaf)
    {
      DMCS_LOG_TRACE(port << ": Send a NULL pointer to OUT_MQ");
      mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
    }
  else
    {
      collect_learned_clauses(xSATSolver->getLearnedClauses());
      const SignatureByCtx& local_sig = boost::get<Tag::Ctx>(*sig);
      SignatureByCtx::const_iterator it;
      Decisionlevel* decision;
      PartialBeliefState* interface_impossible;
      PartialBeliefState* partial_ass;

      *partial_ass = *parent_ass;

      if (trail->empty())
	{
	  *decision = *parent_decision;
	  *interface_impossible = *input;
	} // if (trail->empty())
      else
	{
	  ChoicePointPtr& cp = trail->top();
	  decision = cp->decision; // parent_decision is included
	} // if (trail->empty())

      it = findFirstUndecided(my_id, local_sig, decision, orig_sigs_size);

      if (it == local_sig.end())
	{
	  DMCS_LOG_TRACE(port << ": The parents decided all of my bridge atoms and I am still UNSAT. Send a NULL pointer to OUT_MQ");
	  mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
	}
      else
	{
	  if (!trail->empty())
	    {
	      // Cancel the last push 
	      std::size_t last_decided_atom = decision->last();

	      VecSizeT::const_iterator ot = orig_sigs_size->begin();
	      PartialBeliefState::const_iterator jt = partial_ass->begin();
	      while (last_decided_atom > (*ot))
		{
		  last_decided_atom -= (*ot);
		  ++it;
		  ++jt;
		}

	      setBeliefSet(*jt, last_decided_atom, PartialBeliefSet::DMCS_UNDEF);
	    }

	  // There is an atom to flip. Set the opposite value in input
	  PartialBeliefSet& input_pbs = (*input)[it->ctxId - 1];
	  PartialBeliefSet& partial_ass_pbs = (*partial_ass)[it->ctxId - 1];
	  
	  PartialBeliefSet::TruthVal val = testBeliefSet(input_pbs, it->localId);
	  assert (val != PartialBeliefSet::DMCS_UNDEF);
	  
	  if (val == PartialBeliefSet::DMCS_TRUE)
	    {
	      setBeliefSet(partial_ass_pbs, it->localId, PartialBeliefSet::DMCS_FALSE);
	    }
	  else
	    {
	      setBeliefSet(partial_ass_pbs, it->localId);
	    }
	  
	  decision->setDecisionlevel(gid);
	  
	  ChoicePointPtr cp(new ChoicePoint(input, decision));
	  trail->push(cp);
	}
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
      //DMCS_LOG_DEBUG("input: " << *input);
    }

  //  DMCS_LOG_TRACE(port << ": bs:    " << *bs);

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
      
      PartialBeliefSet& belief = (*bs)[cid];

      if (_aAssignment[i]) 
	{
	  setBeliefSet(belief, loc_it->origId);
	}
      else
	{
	  setBeliefSet(belief, loc_it->origId, PartialBeliefSet::DMCS_FALSE);
	}
    }

  DMCS_LOG_TRACE(port << ": MODEL from SAT: bs = " << *bs);

  // project to my output interface
  project_to(bs, localV, my_id - 1);

  // now put this PartialBeliefState to the SatOutputMessageQueue
  mg->sendModel(bs, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);

  //DMCS_LOG_TRACE(port << ": Solution sent: " << *bs);
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
