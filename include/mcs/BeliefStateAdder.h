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
 * @file   BeliefStateAdder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  12 17:52:36 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef BELIEF_STATE_ADDER_H
#define BELIEF_STATE_ADDER_H

#include <vector>

#include <boost/thread.hpp>

#include "mcs/NewBeliefState.h"
#include "network/ConcurrentMessageQueue.h"

struct BeliefStateAdder
{
  std::vector<NewBeliefState*> results;
  
  void operator()(NewBeliefState* ans)
  {
    results.push_back(ans);
    boost::this_thread::interruption_point();
  }
};

#endif // BELIEF_STATE_ADDER_H

// Local Variables:
// mode: C++
// End:
