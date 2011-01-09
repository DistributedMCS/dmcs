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

namespace dmcs {

RelSatSolver::RelSatSolver(const TheoryPtr& theory_, 
			   const ProxySignatureByLocalPtr& mixed_sig_,
			   std::size_t system_size_,
			   boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : theory(theory_), 
    mixed_sig(mixed_sig_),
    system_size(system_size_),
    mg(mg_),
    xInstance(new SATInstance(std::cerr)),
    xSATSolver(new SATSolver(xInstance, std::cerr, this))
{
  xInstance->readTheory(theory, mixed_sig->size());
}


RelSatSolver::~RelSatSolver()
{
  delete xInstance;
  delete xSATSolver;
}

int
RelSatSolver::solve(const TheoryPtr& theory, std::size_t sig_size)
{ }



void
RelSatSolver::receiveSolution(DomainValue* _aAssignment, int _iVariableCount)
{
#ifdef DEBUG
  std::cerr << "RelSatSolver::receiveSolution" << std::endl;
#endif

  BeliefState* bs = new dmcs::BeliefState(system_size, dmcs::BeliefSet());

  for (int i=0; i<_iVariableCount; i++) 
    {
      assert(_aAssignment[i] != NON_VALUE);
      if (_aAssignment[i]) 
	{
	  dmcs::SignatureByLocal::const_iterator loc_it = mixed_sig->find(i+1);
	  
	  // it must show up in the signature
	  assert (loc_it != mixed_sig->end());
	  
	  std::size_t cid = loc_it->ctxId - 1;
	  
	  // just to be safe
	  assert (cid < system_size);
	  
	  dmcs::BeliefSet& belief = (*bs)[cid];

	  belief.set(loc_it->origId);
	}
    }

  // now put this BeliefState to the SatOutputMessageQueue
  mg->sendModel(bs, 0, MessageQueueFactory::OUT_MQ ,0);

#ifdef DEBUG
  std::cerr << "Solution: " << *bs << std::endl;
#endif
}



int
RelSatSolver::solve()
{
  // read joined input from Joiner
  std::size_t prio;
  BeliefState* input;
  input = mg->recvModel(MessageQueueFactory::JOIN_OUT_MQ, prio);

  // now remove input part of the theory (from last solve)
  xInstance->removeLastInput();

  // then add input to the SATSolver's theory

  // then we can do SAT solving

  relsat_enum eResult = xSATSolver->eSolve();

  return 10;
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
