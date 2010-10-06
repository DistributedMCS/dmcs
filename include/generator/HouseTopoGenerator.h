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
 * @file   HouseTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Sep  17 14:54:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_HOUSE_TOPO_GENERATOR_H
#define GEN_HOUSE_TOPO_GENERATOR_H

#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

class HouseTopoGenerator : public TopologyGenerator
{
public:
  HouseTopoGenerator(NeighborVec2Ptr topo)
    : TopologyGenerator(topo)
  { }

  void
  genNeighborList(std::size_t id)
  {
    if (id == 1)
      {
	genNeighbor(id, id+1);
	genNeighbor(id, id+2);
	return;
      }
    
    std::size_t remainder = (id-1) % 4;
    switch (remainder)
      {
      case 1:
      case 2:
	{
	  genNeighbor(id, id+1);
	  break;
	}
      case 3:
	{
	  genNeighbor(id, id+1);
	  
	  std::size_t child_id = id + ((id/4) - 1)* 4 + 2;
	  if (child_id < system_size)
	    {
	      genNeighbor(id, child_id);
	      genNeighbor(id, child_id+1);
	    }

	  break;
	}
      case 0:
	{
	  genNeighbor(id, id-3);
	  
	  // check this
	  // id + id*4 + 1???
	  // id*5 + 1??? 
	  std::size_t child_id = id + ((id/4) - 1)* 4 + 5;
	  if (child_id < system_size)
	    {
	      genNeighbor(id, child_id);
	      genNeighbor(id, child_id+1);
	    }

	  break;
	}
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_HOUSE_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
