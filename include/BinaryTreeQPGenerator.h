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
 * @file   BinaryTreeQPGenerator.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sat Jan 21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef BINARY_TREE_QP_GENERATOR_H_
#define BINARY_TREE_QP_GENERATOR_H_

#include "Context.h"
#include "QueryPlanGenerator.h"

namespace dmcs { namespace generator {

class BinaryTreeQPGenerator : public QueryPlanGenerator
{
public:
  BinaryTreeQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : QueryPlanGenerator(contexts_, query_plan_)
  { }

  void
  genNeighbors(const ContextPtr& context)
  {
    std::size_t id = context->getContextID();

    std::size_t childId = 2*id;
    if(childId <= context->getSystemSize())
      {
       	genNeighbor(context, childId);
	childId = 2*id + 1;
	if(childId <= context->getSystemSize())
	  {
	    genNeighbor(context, childId);
	  }
      }
  }

  void
  create_interface(const ContextPtr& /* context */)
  { }
};

  } // namespace generator
} // namespace dmcs

#endif // BINARY_TREE_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
