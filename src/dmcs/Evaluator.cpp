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
 * @file   Evaluator.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Jan 23:04:01 18 2012
 *
 * @brief 
 *
 *
 */

#include "dmcs/Evaluator.h"

namespace dmcs {

Evaluator::Evaluator(const InstantiatorWPtr& inst,
		     const NewConcurrentMessageDispatcherPtr d)
  : instantiator(inst),
    md(d)
{
  in_queue = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::EVAL_IN_MQ);
  out_queue = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::EVAL_OUT_MQ);
}



std::size_t
Evaluator::getInQueue()
{
  return in_queue;
}



std::size_t
Evaluator::getOutQueue()
{
  return out_queue;
}



void
Evaluator::operator()()
{
  std::size_t timeout = 0;
  while (1)
    {      
      NewBeliefState* heads = md->receive<NewBeliefState>(NewConcurrentMessageDispatcher::EVAL_IN_MQ, in_queue, timeout);
      solve(heads);
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
