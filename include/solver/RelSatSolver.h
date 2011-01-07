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
 * @file   RelSatSolver.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  4 17:14:38 2011
 * 
 * @brief  
 * 
 * 
 */

#include "BaseSolver.h"
#include "dmcs/Context.h"
#include "dmcs/QueryPlan.h"

#include "relsat-20070104/SATInstance.h"
#include "relsat-20070104/SATSolver.h"

#include <boost/shared_ptr.hpp>

#ifndef REL_SAT_SOLVER_H
#define REL_SAT_SOLVER_H


namespace dmcs {

// Wrapper for relsat
class RelSatSolver : public BaseSolver
{
public:
  RelSatSolver(const TheoryPtr& theory_,
	       const ProxySignatureByLocalPtr& mixed_sig_,
	       std::size_t system_size_);

  ~RelSatSolver();

  int
  solve(const TheoryPtr& theory, std::size_t sig_size);

  int
  solve();

  void
  receiveSolution(DomainValue* _aAssignment, int _iVariableCount);

  void
  show()
  {
    std::cerr << "mixed_sig        = " << mixed_sig << std::endl;
    std::cerr << "mixed_sig.size() = " << mixed_sig->size() << std::endl;
  }

private:
  const TheoryPtr                theory;
  const ProxySignatureByLocalPtr mixed_sig;
  std::size_t                    system_size;

  SATInstance*                   xInstance;
  SATSolver*                     xSATSolver;
};

typedef boost::shared_ptr<RelSatSolver> RelSatSolverPtr;

} // namespace dmcs

#endif // REL_SAT_SOLVER_H

// Local Variables:
// mode: C++
// End:
