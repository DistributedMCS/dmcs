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
 * @file   BaseDMCS.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun May  2 16:42:53 2010
 * 
 * @brief  Base DMCS methods.
 * 
 * 
 */


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "BaseDMCS.h"
#include "ClaspProcess.h"

namespace dmcs {

BaseDMCS::BaseDMCS(const ContextPtr& c, const TheoryPtr& t)
  : ctx(c),
    theory(t)
{ }



BaseDMCS::~BaseDMCS()
{ }



BeliefStateListPtr
BaseDMCS::localSolve(const SignatureByLocal& sig)
{
#ifdef DEBUG
  std::cerr << "Starting local solve..." << std::endl;
#endif

  ClaspProcess cp;
  cp.addOption("-n 0");
  boost::shared_ptr<BaseSolver> solver(cp.createSolver());

  BeliefStateListPtr local_belief_states(new BeliefStateList);

  ///@todo 
  solver->solve(sig, local_belief_states, theory);

#ifdef DEBUG
  std::cerr << "Got " << local_belief_states->size();
  std::cerr << " answers from CLASP" << std::endl;
  std::cerr << "Local belief states from localsolve(): " << std::endl;
  std::cerr << *local_belief_states << std::endl;
#endif
  return local_belief_states;
}


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
