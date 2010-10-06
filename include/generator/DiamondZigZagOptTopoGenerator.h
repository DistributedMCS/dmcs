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
 * @file   DiamondZigZagOptTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Aug  31 8:46:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_DIAMOND_ZIGZAG_OPT_TOPO_GENERATOR_H
#define GEN_DIAMOND_ZIGZAG_OPT_TOPO_GENERATOR_H

#include "BeliefCombination.h"
#include "generator/OptTopologyGenerator.h"

namespace dmcs { namespace generator {

class DiamondZigZagOptTopoGenerator : public OptTopologyGenerator
{
public:
  DiamondZigZagOptTopoGenerator(std::size_t system_size, LocalInterfaceMapPtr lcim)
    : OptTopologyGenerator(system_size, lcim)
  { }

  void
  create_opt_interface(std::size_t id)
  {
    if (id == system_size)
      {
	return;
      }

    std::size_t remainder = id % 3;

    switch(remainder)
      {
      case 1:
	{
	  BeliefStatePtr bs_12 = getInterface(lcim, id, id+1);
	  BeliefStatePtr bs_23 = getInterface(lcim, id+1, id+2);
	  
	  update(bs_12, bs_23);
	  break;
	}
      case 2:
	{
	  BeliefStatePtr bs_23 = getInterface(lcim, id, id+1);
	  BeliefStatePtr bs_13 = getInterface(lcim, id-1, id+1);
	  BeliefStatePtr bs_34 = getInterface(lcim, id+1, id+2);

	  update(bs_23, bs_13);
	  update(bs_23, bs_34);
	  
	  ContextPair p(id-1, id+1);
	  lcim->erase(p);

	  break;
	}
      case 0:
	{
	  BeliefStatePtr bs_34 = getInterface(lcim, id, id+1);
	  BeliefStatePtr bs_24 = getInterface(lcim, id-1, id+1);

	  update(bs_34, bs_24);

	  ContextPair p(id-1, id+1);
	  lcim->erase(p);
	  
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
