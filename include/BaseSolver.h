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
 * @file   BaseSolver.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:34:38 2009
 * 
 * @brief  
 * 
 * 
 */

#include "Theory.h"
#include "BeliefState.h"
#include "Context.h"

#if !defined(BASE_SOLVER_H)
#define BASE_SOLVER_H

namespace dmcs {


/**
 * @brief Base class for solvers.
 */
class BaseSolver
{
 public:
  virtual
  ~BaseSolver()
  {}

  virtual void
  solve(const Context& context, BeliefStatesPtr& belief_states, const TheoryPtr& theory, const BeliefStatePtr& V) = 0;
};

} // namespace dmcs

#endif // BASE_SOLVER_H

// Local Variables:
// mode: C++
// End:
