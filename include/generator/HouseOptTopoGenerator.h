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
 * @file   HouseOptTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  20 11:30:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_HOUSE_OPT_TOPO_GENERATOR_H
#define GEN_HOUSE_OPT_TOPO_GENERATOR_H

#include "dmcs/BeliefCombination.h"
#include "generator/OptTopologyGenerator.h"

namespace dmcs { namespace generator {

class HouseOptTopoGenerator : public OptTopologyGenerator
{
public:
  HouseOptTopoGenerator(std::size_t system_size, LocalInterfaceMapPtr lcim)
    : OptTopologyGenerator(system_size, lcim)
  { }

  void
  create_opt_interface(std::size_t id)
  {
    if (id == 1)
      {
	NewBeliefStatePtr bs_12 = getInterface(lcim, id-1, id);
	NewBeliefStatePtr bs_23 = getInterface(lcim, id, id+1);
	(*bs_12) = (*bs_12) | (*bs_23);
	return;
      }
    
    std::size_t remainder = id % 4;

    switch (remainder)
      {
      case 2:
	{
	  std::size_t house_id = (id + 2)/4;
	  std::size_t parent_id = 1;
	  if (house_id % 2 == 0)
	    {
	      parent_id = 2*(((id - 2)/4) + 1);
	    }
	  else
	    {
	      if (id != 2)
		{
		  parent_id = 2*(((id - 5)/4) + 1) + 1;
		}
	    }

	  NewBeliefStatePtr bs_23 = getInterface(lcim, id-1, id);
	  NewBeliefStatePtr bs_p3 = getInterface(lcim, parent_id-1, id);
	  NewBeliefStatePtr bs_30 = getInterface(lcim, id, id+1);

	  (*bs_23) = (*bs_23) | (*bs_p3);
	  (*bs_23) = (*bs_23) | (*bs_30);

	  ContextPair p(parent_id-1, id);
	  lcim->erase(p);

	  break;
	}
      case 3:
	{
	  NewBeliefStatePtr bs_30 = getInterface(lcim, id-1, id);
	  NewBeliefStatePtr bs_01 = getInterface(lcim, id, id+1);

	  (*bs_30) = (*bs_30) | (*bs_01);
	  break;
	}
      case 0:
	{
	  NewBeliefStatePtr bs_01 = getInterface(lcim, id-1, id);
	  NewBeliefStatePtr bs_12 = getInterface(lcim, id, id-3);

	  (*bs_01) = (*bs_01) | (*bs_12);

	  ContextPair p(id, id-3);
	  lcim->erase(p);

	  std::size_t child_id = id + ((id/4) - 1)*4 + 2;
	  if (child_id < system_size)
	    {
	      NewBeliefStatePtr bs_p2 = getInterface(lcim, id-1, child_id-1);
	      NewBeliefStatePtr bs_23 = getInterface(lcim, child_id-1, child_id);

	      (*bs_p2) = (*bs_p2) | (*bs_23);
	    }

	  break;
	}
      case 1:
	{
	  std::size_t child_id = id + ((id/4) - 1)*4 + 5;
	  if (child_id < system_size)
	    {
	      NewBeliefStatePtr bs_p2 = getInterface(lcim, id-1, child_id-1);
	      NewBeliefStatePtr bs_23 = getInterface(lcim, child_id-1, child_id);
	      
	      (*bs_p2) = (*bs_p2) | (*bs_23);
	    }
	  break;
	}
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_DIAMOND_OPT_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
