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
 * @file   OptTopologyGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu  Sep 16 20:39:00 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_OPT_TOPOLOGY_GENERATOR_H
#define GEN_OPT_TOPOLOGY_GENERATOR_H

#include <vector>
#include <boost/shared_ptr.hpp>

#include "generator/TopologyGenerator.h"
#include "mcs/BeliefState.h"

namespace dmcs { namespace generator {

typedef std::pair<std::size_t, std::size_t> ContextPair;
typedef std::pair<ContextPair, BeliefStatePtr> LocalInterfacePair;
typedef std::map<ContextPair, BeliefStatePtr> LocalInterfaceMap;
typedef boost::shared_ptr<LocalInterfaceMap> LocalInterfaceMapPtr;


inline BeliefStatePtr
getInterface(LocalInterfaceMapPtr lcim, std::size_t from, std::size_t to)
{
  ContextPair cp(from, to);
  LocalInterfaceMap::const_iterator lcim_it = lcim->find(cp);

  assert (lcim_it != lcim->end());

  return lcim_it->second;
}


inline void
putInterface(LocalInterfaceMapPtr lcim, std::size_t from, std::size_t to, const BeliefStatePtr& bs)
{
  ContextPair cp(from, to);
  LocalInterfaceMap::iterator lcim_it = lcim->find(cp);
  assert (lcim_it != lcim->end());

  lcim_it->second = bs;
}


class OptTopologyGenerator
{
public:
  OptTopologyGenerator(std::size_t system_size_, LocalInterfaceMapPtr lcim_)
    : system_size(system_size_), lcim(lcim_)
    { }

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
  LocalInterfaceMapPtr lcim;
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_OPT_TOPOLOGY_GENERATOR_H

// Local Variables:
// mode: C++
// End:
