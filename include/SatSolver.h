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
 * @file   SatSolver.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:55:18 2009
 * 
 * @brief   
 * 
 */

#ifndef SAT_SOLVER_H
#define SAT_SOLVER_H

#include "BaseSolver.h"


namespace dmcs {

class Process;

template <typename Builder, typename Parser, typename ParserGrammar>
class SatSolver : public BaseSolver
{
private:
  Process& proc;

public:
  /// Ctor.
  SatSolver(Process& p);

  void
  solve(const Context& context, BeliefStatesPtr& belief_states, const TheoryPtr& theory, const BeliefStatePtr& V);

};

} // namespace dmcs

#endif // SAT_SOLVER_H

#include "SatSolver.tcc"

// Local Variables:
// mode: C++
// End:
