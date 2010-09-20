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
 * @file   TopologyGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Aug  31 8:19:00 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_TOPOLOGY_GENERATOR_H
#define GEN_TOPOLOGY_GENERATOR_H

#include <vector>
#include <boost/shared_ptr.hpp>

#define RANDOM_TOPOLOGY            0
#define DIAMOND_DOWN_TOPOLOGY      1
#define DIAMOND_ARBITRARY_TOPOLOGY 2
#define DIAMOND_ZIGZAG_TOPOLOGY    3
#define PURE_RING_TOPOLOGY         4
#define RING_EDGE_TOPOLOGY         5
#define BINARY_TREE_TOPOLOGY       6
#define HOUSE_TOPOLOGY             7
#define MULTIPLE_RING_TOPOLOGY     8

namespace dmcs { namespace generator {

typedef std::vector<std::size_t> NeighborVec;
typedef boost::shared_ptr<NeighborVec> NeighborVecPtr;
typedef std::vector<NeighborVecPtr> NeighborVec2;
typedef boost::shared_ptr<NeighborVec2> NeighborVec2Ptr;


class TopologyGenerator
{
public:
  TopologyGenerator(NeighborVec2Ptr topo_)
    : topo(topo_), system_size(topo_->size())
  { }

  void
  generate()
  {
    for (std::size_t i = 1; i <= system_size; ++i)
      {
	genNeighborList(i);
      }
  }

  virtual void
  genNeighborList(std::size_t context_id) = 0;

  void
  genNeighbor(std::size_t from, std::size_t to)
  {
    (*topo)[from-1]->push_back(to);
  }

protected:
  NeighborVec2Ptr topo;
  std::size_t system_size;
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_TOPOLOGY_GENERATOR_H

// Local Variables:
// mode: C++
// End:
