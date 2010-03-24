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
 * @file   MultipleRingOptQPGenerator.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Tue Jan 21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef MULTIPLE_RING_OPT_QP_GENERATOR_H_
#define MULTIPLE_RING_OPT_QP_GENERATOR_H_

#include "Context.h"
#include "MultipleRingQPGenerator.h"


namespace dmcs { namespace generator {

class MultipleRingOptQPGenerator : public  MultipleRingQPGenerator
{
public:
  MultipleRingOptQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    :  MultipleRingQPGenerator(contexts_, query_plan_)
  { }

  void
  create_interface(const ContextPtr& context)
  { 
    const std::size_t id = context->getContextID();
    const std::size_t earLoopSize = (context->getSystemSize() + 3) / 3;
    const std::size_t secondLoopJump = earLoopSize-1;
    const std::size_t firstLoopEnd = 2 +earLoopSize + earLoopSize -3;

    if( (id == firstLoopEnd)||( id ==context->getSystemSize()))
      {
	return;
      }
    const BeliefStatePtr bs_2a = query_plan->getInterface(firstLoopEnd, 3);
    const BeliefStatePtr bs_2b = query_plan->getInterface(earLoopSize+1, 1);
    const BeliefStatePtr bs_2c = query_plan->getInterface(context->getSystemSize(), earLoopSize);

    if(id == earLoopSize +1)
      {
	BeliefStatePtr bs_11 = query_plan->getInterface(id, id+secondLoopJump);	    
	update(bs_11, bs_2a);
	update(bs_11, bs_2b);
	update(bs_11, bs_2c);
	query_plan->putInterface(id, id+secondLoopJump, bs_11);

      }
    else
      {
	BeliefStatePtr bs_12 = query_plan->getInterface(id, id+1);	    
	update(bs_12, bs_2a);
	update(bs_12, bs_2b);
	update(bs_12, bs_2c);
	query_plan->putInterface(id, id+1, bs_12);
      }

    if(id == 2)
      {
	BeliefStatePtr bs_11 = query_plan->getInterface(id, id+earLoopSize);	    
	update(bs_11, bs_2a);
	update(bs_11, bs_2b);
	update(bs_11, bs_2c);
	query_plan->putInterface(id, id+earLoopSize, bs_11);
      }
    if(id == 1)
      {
	query_plan->remove_connection(firstLoopEnd, 3);
	query_plan->remove_connection(earLoopSize+1, 1);
	query_plan->remove_connection(context->getSystemSize(), earLoopSize);
      }	 
  }
};

  } // namespace generator
} // namespace dmcs

#endif // MULTIPLE_RING_OPT_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
