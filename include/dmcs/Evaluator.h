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
#include "network/ConcurrentMessageQueue.h"

namespace dmcs {

class Instantiator;

class Evaluator
{
public:
	// theory is accessed via inst
	// (derived classes can cache it here for better performance)
  Evaluator(/*const TheoryPtr& t, */const InstantiatorWPtr& inst);

	virtual
  ~Evaluator();

	// can be nonvirtual
	// should never be overridden (final)
  ConcurrentMessageQueue*
  getInQueue();

	// can be nonvirtual
	// should never be overridden (final)
  ConcurrentMessageQueue*
  getOutQueue();

	// this starts the evaluator thread
	// 
	// perhaps (after implementing 2 or 3 different contexts)
	// we will do the thread here and make some other low-level functionality virtual
	// (I would let this be here for now, as we don't have much experience with
	// implementing such contexts yet)
	virtual void
  solve() = 0;

private:
  //TheoryPtr theory; see Instantiator.h why this should not be specified here
  InstantiatorWPtr instantiator;
  ConcurrentMessageQueue* in_queue;
  ConcurrentMessageQueue* out_queue;
};

} // namespace dmcs

#endif // EVALUATOR_H
