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
 * @file   DiamondTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Aug  31 8:46:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_DIAMOND_TOPO_GENERATOR_H
#define GEN_DIAMOND_TOPO_GENERATOR_H

#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

class DiamondTopoGenerator : public TopologyGenerator
{
public:
  DiamondTopoGenerator(std::size_t system_size, NeighborVecListPtr topo)
    : TopologyGenerator(system_size, topo)
  { }

  void
  genNeighborList(std::size_t id)
  {
    if (id == system_size)
      {
	return;
      }

    std::size_t remainder = id % 3;
    switch (remainder)
      {
      case 1:
	{
	  genNeighbor(context, id+1);
	  genNeighbor(context, id+2);
	  break;
	}
      case 0:
	{
	  genNeighbor(context, id+2);
	  break;
	}
      case 2:
	{
	  genNeighbor(context, id+2);
	  break;
	}	
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_DIAMONDTOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
