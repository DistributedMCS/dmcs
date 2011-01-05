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

RelSatSolver::RelSatSolver(const ContextPtr& context_,
			   const TheoryPtr& theory_, 
			   const SignatureVecPtr& global_sigs_)
  : context(context_),
    theory(theory_), 
    global_sigs(global_sigs_),
    xInstance(std::cerr),
    xSATSolver(&xInstance, std::cerr, context->getSystemSize()),
    sig_size(0)
{ }



int
RelSatSolver::solve(const TheoryPtr& theory, std::size_t sig_size)
{
  relsat_enum eResult = xSATSolver.eSolve();
}



int
RelSatSolver::solve()
{
  // read the query from QueryMessageQueue

  // from this we get (invoker), (conflict)

  // then we can create guessing signature based on the interface from
  // query_plan(invoker, myself)

  std::size_t invoker = 0;
  BeliefStatePtr conflict(new BeliefState);

  std::size_t my_id = context->getContextID();

  // compute the size of the mixed signature, just once.
  if (sig_size == 0)
    {
      
    }

  if (!xInstance.hasTheory())
    {
      xInstance.readTheory(theory, sig_size);
    }

  return solve(theory, sig_size);
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
