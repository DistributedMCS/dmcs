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
#include <list>

#include <boost/shared_ptr.hpp>

#include "BeliefState.h"

namespace dmcs { namespace generator {

typedef std::vector<std::size_t> NeighborVec;
typedef boost::shared_ptr<NeighborVec> NeighborVecPtr;
typedef std::vector<NeighborVecPtr> NeighborVec2;
typedef boost::shared_ptr<NeighborVec2> NeighborVec2Ptr;

typedef std::pair<std::size_t, std::size_t> ContextPair;
typedef std::pair<ContextPair, BeliefStatePtr> LocalInterfacePair;
typedef std::map<ContextPair, BeliefStatePtr> LocalInterfaceMap;
typedef boost::shared_ptr<LocalInterfaceMap> LocalInterfaceMapPtr;


class TopologyGenerator
{
public:
  TopologyGenerator(NeighborVec2Ptr topo_)
    : topo(topo_), system_size(topo_->size())
  { }

  TopologyGenerator(NeighborVec2Ptr topo_, 
		    LocalInterfaceMapPtr lcim_)
    : topo(topo_), lcim(lcim_), system_size(topo->size())
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

  void
  create_opt_interface()
  {
    for (std::size_t i = system_size; i > 0; --i)
      {
	create_opt_interface(i);
      }
  }

  virtual void
  create_opt_interface(std::size_t id) = 0;

protected:
  std::size_t system_size;
  NeighborVec2Ptr topo;
  LocalInterfaceMapPtr lcim;
};


inline
BeliefStatePtr 
getInterface(LocalInterfaceMapPtr lcim, std::size_t from, std::size_t to)
{
  ContextPair cp(from, to);
  LocalInterfaceMap::iterator it = lcim->find(cp);
  
  return it->second;
}

  } // namespace generator
} // namespace dmcs

#endif // GEN_TOPOLOGY_GENERATOR_H

// Local Variables:
// mode: C++
// End:
