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
 * @file   DiamondZigZagQPGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  21 11:49:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DIAMOND_ZIGZAG_QUERY_PLAN_GENERATOR_H_
#define DIAMOND_ZIGZAG_QUERY_PLAN_GENERATOR_H_

#include "Context.h"
#include "QueryPlanGenerator.h"

namespace dmcs { namespace generator {


class DiamondZigZagQPGenerator : public QueryPlanGenerator
{
public:
  DiamondZigZagQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : QueryPlanGenerator(contexts_, query_plan_)
  { }

  void
  genNeighbors(const ContextPtr& context)
  {
    std::size_t id = context->getContextID();

    if (id == context->getSystemSize())
      {
	return;
      }

    int remainder = id % 3;
    switch (remainder)
      {
      case 1:
      case 2:
	genNeighbor(context, id+1);
	genNeighbor(context, id+2);
	break;
      case 0:
	genNeighbor(context, id+1);
	break;
      }
  }

  void
  create_interface(const ContextPtr& context)
  {
  }
};


  } // namespace generator
} // namespace dmcs

#endif // DIAMOND_ZIGZAG_QUERY_PLAN_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
