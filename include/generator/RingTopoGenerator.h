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
 * @file   RingTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Sep  17 14:00:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_RING_TOPO_GENERATOR_H
#define GEN_RING_TOPO_GENERATOR_H

#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

class RingTopoGenerator : public TopologyGenerator
{
public:
  RingTopoGenerator(NeighborVec2Ptr topo)
    : TopologyGenerator(topo)
  { }

  void
  genNeighborList(std::size_t id)
  {
    if (id == system_size)
      {
	genNeighbor(id, 1);
      }
    else
      {
	genNeighbor(id, id+1);
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_RING_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
