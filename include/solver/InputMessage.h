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
 * @file   InputMessage.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Dec  6 11:20:20 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef INPUT_MESSAGE_H
#define INPUT_MESSAGE_H

#include <boost/shared_ptr.hpp>

#include "solver/BaseSolverState.h"

namespace dmcs {

struct InputMessage
{
  BridgeHeads* bridge_heads; // bridge heads 
  BaseSolverStatePtr state;  // the internal state of the solver
  std::size_t count;         // the number of models we want from the solver
};

} // INPUT_MESSAGE_H

#endif

// Local Variables:
// mode: C++
// End:
