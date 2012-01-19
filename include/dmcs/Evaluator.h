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
 * @file   Evaluator.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Nov 17:22:00 28 2011
 *
 * @brief 
 *
 *
 */


#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "dmcs/AbstractContext.h"
#include "network/NewConcurrentMessageDispatcher.h"

namespace dmcs {

class Instantiator;

class Evaluator
{
public:
  struct GenericOptions
  {
    GenericOptions();
    virtual ~GenericOptions();

    bool includeFacts;
  };

  Evaluator(const InstantiatorWPtr& inst,
	    const NewConcurrentMessageDispatcherPtr d);

  virtual ~Evaluator();
  
  std::size_t
  getInQueue();

  std::size_t
  getOutQueue();

  // this starts the evaluator thread
  void
  operator()();

protected:
  virtual void
  solve(NewBeliefState* heads) = 0;

protected:
  InstantiatorWPtr instantiator;
  NewConcurrentMessageQueueDispatcherPtr md;
  std::size_t in_queue;                      // id to the ConcurrentMessageQueue provided by the MessageDispatcher
  std::size_t out_queue;
};

} // namespace dmcs

#endif // EVALUATOR_H

// Local Variables:
// mode: C++
// End:
