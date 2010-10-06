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
 * @file   RingEdgeTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Sep  17 14:28:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_RING_EDGE_TOPO_GENERATOR_H
#define GEN_RING_EDGE_TOPO_GENERATOR_H

#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

class RingEdgeTopoGenerator : public TopologyGenerator
{
public:
  RingEdgeTopoGenerator(NeighborVec2Ptr topo)
    : TopologyGenerator(topo)
  { }

  void
  genNeighborList(std::size_t id)
  {
    std::size_t next;

    // firstly contribute to the cycle
    if (id == system_size)
      {
	next = 1;
      }
    else
      {
	next = id+1;
      }

    genNeighbor(id, next);

    // then generate an additional edge with a probability of 1/2
    if (rand() % 2)
      {
	std::size_t other;
	do
	  {
	    other = (rand() % system_size) + 1;
	  }
	while (other == next);
	
	genNeighbor(id, other);
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_RING_EDGE_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
