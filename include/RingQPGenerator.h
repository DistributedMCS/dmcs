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
 * @file   RingQPGenerator.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sat Jan  17 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef RING_QP_GENERATOR_H_
#define RING_QP_GENERATOR_H_

#include "Context.h"
#include "QueryPlanGenerator.h"

namespace dmcs { namespace generator {

class RingQPGenerator : public QueryPlanGenerator
{
public:
  RingQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : QueryPlanGenerator(contexts_, query_plan_)
  { }

  void
  genNeighbors(const ContextPtr& context)
  {
    std::size_t id = context->getContextID();

    if ((std::size_t)id == context->getSystemSize())
      {
       	genNeighbor(context, 1);
      }
    else
      {
	genNeighbor(context, id+1);
      }
  }

  void
  create_interface(const ContextPtr& context)
  { }
};


  } // namespace generator
} // namespace dmcs

#endif // Ring_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
