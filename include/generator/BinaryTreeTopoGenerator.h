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
 * @file   BinaryTreeTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  20 9:38:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_BINARY_TREE_TOPO_GENERATOR_H
#define GEN_BINARY_TREE_TOPO_GENERATOR_H

#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

class BinaryTreeTopoGenerator : public TopologyGenerator
{
public:
  BinaryTreeTopoGenerator(NeighborVec2Ptr topo)
    : TopologyGenerator(topo)
  { }

  void
  genNeighborList(std::size_t id)
  {
    std::size_t child_id = 2*id;
    if (child_id <= system_size)
      {
	genNeighbor(id, child_id);

	++child_id;
	if (child_id <= system_size)
	  {
	    genNeighbor(id, child_id);
	  }
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_BINARY_TREE_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
