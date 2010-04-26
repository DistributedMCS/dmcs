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
 * @file   HouseQPGenerator.h
 * @author Seif El-Din Bairakdar <bairakdar@kr.tuwien.ac.at>
 * @date   Sat Jan  21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef HOUSE_QP_GENERATOR_H_
#define HOUSE_QP_GENERATOR_H_

#include "Context.h"
#include "QueryPlanGenerator.h"


namespace dmcs { namespace generator {


class HouseQPGenerator : public QueryPlanGenerator
{
public:
  HouseQPGenerator(ContextsPtr& contexts_, QueryPlanPtr& query_plan_)
    : QueryPlanGenerator(contexts_, query_plan_)
  { }

  void
  genNeighbors(const ContextPtr& context)
  {
    std::size_t id = context->getContextID();
    if ((std::size_t)id == 1)
      {
	genNeighbor(context, id+1);
	genNeighbor(context, id+2);
	return;
      }

    int remainder = (id-1) % 4;
    switch (remainder)
      {
      case 1:
	genNeighbor(context, id+1);
	break;
      case 2:
	genNeighbor(context, id+1);
	break;
      case 3:
	{
	  genNeighbor(context, id+1);
	  std::size_t childId = id + ((id/4)-1)*4 + 2;
	  if(childId < context->getSystemSize())
	    {
	      genNeighbor(context, childId);	    
	      genNeighbor(context, childId + 1);	    
	    }
	}
	break;     
      case 0:
	{
	  genNeighbor(context, id-3);
	  std::size_t childId = id + ((id/4)-1)*4 + 5;
	  if(childId < context->getSystemSize())
	    {
	      genNeighbor(context, childId);	    
	      genNeighbor(context, childId + 1);	    
	    }
	}
	break;
      }
  }

  void
  create_interface(const ContextPtr& /* context */)
  { }
};

  } // namespace generator
} // namespace dmcs

#endif // HOUSE_QP_GENERATOR_H_

// Local Variables:
// mode: C++
// End:
