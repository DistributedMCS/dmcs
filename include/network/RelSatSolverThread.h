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
 * @file   RelSatSolverThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:14:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef RELSATSOLVER_THREAD_H
#define RELSATSOLVER_THREAD_H

#include "solver/RelSatSolver.h"

namespace dmcs {

class RelSatSolverThread
{
public:
  RelSatSolverThread(const RelSatSolverPtr& rss);

  void
  operator()();

private:
  RelSatSolverPtr relsatsolver;
};

} // namespace dmcs


#endif // RELSATSOLVER_THREAD_H

// Local Variables:
// mode: C++
// End:
