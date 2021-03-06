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

#include <limits>
#include "solver/Evaluator.h"

namespace dmcs {

Evaluator::GenericOptions::GenericOptions()
  : includeFacts(true)
{ }



Evaluator::GenericOptions::~GenericOptions()
{ }



Evaluator::Evaluator(const InstantiatorWPtr& inst)
  : instantiator(inst),
    models_counter(std::numeric_limits<std::size_t>::max()),
    current_heads(new Heads(NULL, 0, 0)),
    initialized(false)
{ }



Evaluator::~Evaluator()
{ }



std::size_t
Evaluator::getInQueue() const
{
  return in_queue;
}



std::size_t
Evaluator::getOutQueue() const
{
  return out_queue;
}



std::size_t
Evaluator::getModelsCounter() const
{
  return models_counter;
}


void
Evaluator::startup(std::size_t ctx_id,
		   BeliefTablePtr btab,
		   NewConcurrentMessageDispatcherPtr md)
{
  while (1)
    {
      int timeout = 0;
      Heads* heads = md->receive<Heads>(NewConcurrentMessageDispatcher::EVAL_IN_MQ, in_queue, timeout);
      
      if (heads != NULL)
	{
	  solve(ctx_id, heads, btab, md);
	}
      else
	{
	  break;
	}
    }
}


void
Evaluator::init_mqs(NewConcurrentMessageDispatcherPtr md)
{
  if (!initialized)
    {
      in_queue = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::EVAL_IN_MQ);
      out_queue = md->createAndRegisterMQ(NewConcurrentMessageDispatcher::EVAL_OUT_MQ);
      initialized = true;
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
