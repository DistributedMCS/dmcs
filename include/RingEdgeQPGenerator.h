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
 * @date   Sat Jan  21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef RING_EDGE_QP_GENERATOR_H_
#define RING_EDGE_QP_GENERATOR_H_

#include "Context.h"
#include "QueryPlanGenerator.h"

namespace dmcs { namespace generator {

class RingEdgeQPGenerator : public QueryPlanGenerator
{
public:
  RingEdgeQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : QueryPlanGenerator(contexts_, query_plan_)
  { }

  void
  genNeighbors(const ContextPtr& context)
  {
    std::size_t id = context->getContextID();
    int nextId = 0;
    if ((std::size_t)id == context->getSystemSize())
      {
       	genNeighbor(context, 1);
	nextId = 1;
      }
    else
      {
	genNeighbor(context, id+1);
	nextId = id+1;
      }
    // we randomly assign edges with a probability of 50 %
    if(rand() % 2)
      {
	int vertexId = nextId;
	// this loop is added to prevent having repeated edges
	while(vertexId == nextId)
	  {
	    vertexId = (rand()% context->getSystemSize()) +1;
	  }
	genNeighbor(context, vertexId);
      }
  }

  void
  create_interface(const ContextPtr& context)
  { }
};

  } // namespace generator
} // namespace dmcs

#endif // Ring_EDGE_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
