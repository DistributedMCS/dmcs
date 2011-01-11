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
 * @file   RelSatSolverThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  3 15:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "network/RelSatSolverThread.h"

namespace dmcs {

RelSatSolverThread::RelSatSolverThread(const RelSatSolverPtr& relsatsolver_)
  : relsatsolver(relsatsolver_)
{
  std::cerr << "RelSatSolverThread::ctor" << std::endl;
}

void
RelSatSolverThread::operator()()
{
#ifdef DEBUG
  std::cerr << "RelSatSolverThread::operator()()" << std::endl;
#endif

  relsatsolver->solve();
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
