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
 * @file   BaseSolverState.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Dec  6 11:06:30 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_SOLVER_STATE_H
#define BASE_SOLVER_STATE_H

#include <boost/shared_ptr.hpp>
#include <bm/bm.h>

namespace dmcs {

// This is a base class for keeping track of the internal state of a solver.
// The purpose is to be able to store the state somewhere and later, 
// to continue computing models from a state.
class BaseSolverState
{
};

typedef boost::shared_ptr<BaseSolverState> BaseSolverStatePtr;
typedef bm::bvector<> BridgeHeads;

} // BASE_SOLVER_STATE_H

#endif

// Local Variables:
// mode: C++
// End:
