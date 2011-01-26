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


#include "dmcs/BeliefCombination.h"
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
			   //			   const ProxySignatureByLocalPtr& mixed_sig_,
			   const HashedBiMapPtr& co,
			   std::size_t system_size_,
			   MessagingGatewayBCPtr& mg_,
			   ConcurrentMessageQueuePtr& dsn,
			   ConcurrentMessageQueuePtr& srn)
  : is_leaf(il),
    my_id(my_id_),
    theory(theory_), 
    sig(sig_),
    localV(localV_),
    c2o(co),
    //    mixed_sig(mixed_sig_),
    system_size(system_size_),
    mg(mg_),
    dmcs_sat_notif(dsn),
    sat_router_notif(srn),
    learned_conflicts(new ConflictVec2),
    xInstance(new SATInstance(std::cerr)),
    xSATSolver(new SATSolver(xInstance, std::cerr, this))
{
  xInstance->readTheory(theory, sig->size());

  // prepare storage for conflicts learned by the solver
  std::size_t no_nbs = c2o->size();
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      ConflictVecPtr cv(new ConflictVec);
      learned_conflicts->push_back(cv);
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

  /*DMCS_LOG_TRACE("input:    " << *input);
  DMCS_LOG_TRACE("context:  " << it->ctxId - 1);
  DMCS_LOG_TRACE("bset:     " << b);
  DMCS_LOG_TRACE("localid:  " << lid);

  DMCS_LOG_TRACE("Adding unit clause " << ucl << " to local theory.");*/

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

  return true;
}



void
RelSatSolver::import_conflicts(const ConflictVecPtr& conflicts)
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
  // wait for conflict and partial_ass from Handler
  DMCS_LOG_TRACE(" Fresh solving. Wait for a message from DMCS");
  ConflictNotification* cn;
  void *ptr         = static_cast<void*>(&cn);
  unsigned int p    = 0;
  std::size_t recvd = 0;

  dmcs_sat_notif->receive(ptr, sizeof(cn), recvd, p);

  if (ptr && cn)
    {
      ConflictVecPtr conflicts            = cn->conflicts;
      PartialBeliefState* new_partial_ass = cn->partial_ass;

      import_conflicts(conflicts);

      ///@todo what happens with conflict and new_partial_ass here?

      DMCS_LOG_TRACE(" Got a message from DMCS. conflict = " << *conflicts << ". new_partial_ass = " << *new_partial_ass);

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
	      xInstance->removeInput();
	      if (!prepare_input())
		{
		  // send a NULL model to OUT_MQ to inform OutputThread
		  mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);
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
RelSatSolver::collect_learned_clauses(ClauseList& learned_clauses)
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

  for (int i = 0; i < learned_clauses.iClauseCount(); ++i)
    {
      ::Clause* c = learned_clauses.pClause(i);
      Conflict* conflict = new Conflict(system_size, PartialBeliefSet());
      PartialBeliefSet* neighbor_ref;

      std::size_t from_neighbor = 0;

      for (int j = 0; j < c->iVariableCount(); ++j)
	{
	  int atom = back_2_lit(c->eConstrainedLiteral(j));

	  assert (atom != 0);
	  std::size_t abs_atom = std::abs(atom);

	  const SignatureByLocal& sig_by_local    = boost::get<Tag::Local>(*sig);
	  SignatureByLocal::const_iterator loc_it = sig_by_local.find(abs_atom);

	  assert (loc_it != sig_by_local.end());

	  std::size_t orig_ctx = loc_it->ctxId;

	  if (orig_ctx == my_id)
	    {
	      // We don't care about conflict in the local context
	      from_neighbor = 0;
	      break;
	    }
	  else if (from_neighbor == 0)
	    {
	      from_neighbor = orig_ctx;
	      neighbor_ref  = &((*conflict)[from_neighbor - 1]);
	    }
	  else if (from_neighbor != orig_ctx)
	    {
	      // this learned clause includes atoms from different
	      // neighbors, we don't collect it
	      from_neighbor = 0;
	      break;
	    }

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

	  ConflictVecPtr& storage_ref = (*learned_conflicts)[neighbor_offset];
	  storage_ref->push_back(conflict);
	}
    }
}


void
RelSatSolver::receiveUNSAT()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (is_leaf)
    {
      // send a NULL pointer to the SatOutputMessageQueue
      mg->sendModel(0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
    }
}



void
RelSatSolver::receiveSolution(DomainValue* _aAssignment, int _iVariableCount)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

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

  DMCS_LOG_TRACE("bs:    " << *bs);

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

  //DMCS_LOG_TRACE("After adding result: bs = " << *bs);

  // project to my output interface
  project_to(bs, localV, my_id - 1);

  // now put this PartialBeliefState to the SatOutputMessageQueue
  mg->sendModel(bs, 0, ConcurrentMessageQueueFactory::OUT_MQ ,0);

  //DMCS_LOG_TRACE("Solution sent: " << *bs);
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
