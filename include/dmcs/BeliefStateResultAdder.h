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
 * @file   BeliefStateResultAdder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  19 15:10:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef BELIEF_STATE_RESULT_ADDER_H
#define BELIEF_STATE_RESULT_ADDER_H

#include "mcs/NewBeliefState.h"
#include "mcs/Heads.h"
#include "network/NewConcurrentMessageDispatcher.h"

namespace dmcs {

struct BeliefStateResultAdder
{
  std::size_t out_queue;
  NewConcurrentMessageDispatcherPtr md;
  Heads* heads;

  BeliefStateResultAdder(std::size_t oq, 
			 NewConcurrentMessageDispatcherPtr d,
			 Heads* h)
    : out_queue(oq), md(d), heads(h)
  { }

  void
  operator()(NewBeliefState* ans)
  {
    HeadsBeliefStatePair* res = new HeadsBeliefStatePair();
    res->first = heads;
    res->second = ans;

    int timeout = 0;
    md->send(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, out_queue, res, timeout);
  }
};

} // namespace dmcs

#endif // BELIEF_STATE_RESULT_ADDER_H

// Local Variables:
// mode: C++
// End:
