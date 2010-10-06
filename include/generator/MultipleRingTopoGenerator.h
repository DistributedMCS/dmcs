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
 * @file   MultipleRingTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  20 14:46:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_MULTIPLE_RING_TOPO_GENERATOR_H
#define GEN_MULTIPLE_RING_TOPO_GENERATOR_H

#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

class MultipleRingTopoGenerator : public TopologyGenerator
{
public:
  MultipleRingTopoGenerator(NeighborVec2Ptr topo)
    : TopologyGenerator(topo)
  { }

  void
  genNeighborList(std::size_t id)
  {
    std::size_t earLoopSize = (system_size + 3) / 3;
    std::size_t secondLoopJump = earLoopSize - 1;
    std::size_t firstLoopEnd = 2 + earLoopSize + earLoopSize - 3;

    if (id == 2) 
      {
	genNeighbor(id, id+1);
	genNeighbor(id, id + earLoopSize);
	return;
      }

    if (id == earLoopSize + 1)
      {
	genNeighbor(id, 1);
	genNeighbor(id, id+secondLoopJump);
	return;
      }

    if (id == system_size)
      {
	genNeighbor(id, earLoopSize);
	return;
      }

    if (id == firstLoopEnd) 
      {
	genNeighbor(id, 3);
	return;
      }

    genNeighbor(id, id+1);
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_MULTIPLE_RING_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
