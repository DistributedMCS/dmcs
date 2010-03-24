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
 * @file   DiamondQPGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  21 11:00:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DIAMOND_OPT_QUERY_PLAN_GENERATOR_H_
#define DIAMOND_OPT_QUERY_PLAN_GENERATOR_H_

#include "BeliefCombination.h"
#include "Context.h"
#include "DiamondQPGenerator.h"

namespace dmcs { namespace generator {

class DiamondOptQPGenerator : public DiamondQPGenerator
{
public:
  DiamondOptQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : DiamondQPGenerator(contexts_, query_plan_)
  { }

  void
  create_interface(const ContextPtr& context)
  { 
    std::size_t id = context->getContextID();

    if (id == context->getSystemSize())
      {
	return;
      }

    int remainder = id % 3;
    switch(remainder)
      {
      case 1:
	{
	  BeliefStatePtr bs_12 = query_plan->getInterface1(id, id+1);
	  BeliefStatePtr bs_13 = query_plan->getInterface1(id, id+2);
	  BeliefStatePtr bs_24 = query_plan->getInterface1(id+1, id+3);
	  BeliefStatePtr bs_34 = query_plan->getInterface1(id+2, id+3);
	  
	  update(bs_12, bs_24);
	  update(bs_13, bs_34);
	  
	  query_plan->putInterface(id, id+1, bs_12);
	  query_plan->putInterface(id, id+2, bs_13);
	  break;
	}
      case 2:
	{
	  BeliefStatePtr bs_24 = query_plan->getInterface1(id, id+2);
	  BeliefStatePtr bs_34 = query_plan->getInterface1(id+1, id+2);

	  update(bs_24, bs_34);
	  
	  query_plan->putInterface(id, id+2, bs_24);
	  query_plan->putInterface(id+1, id+2, bs_24);
	  break;
	}
      }
  }
};


  } // namespace generator
} // namespace dmcs

#endif // DIAMOND_OPT_QUERY_PLAN_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
