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
 * @file   RingOptTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Sep  17 10:17:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_RING_OPT_TOPO_GENERATOR_H
#define GEN_RING_OPT_TOPO_GENERATOR_H

#include "generator/OptTopologyGenerator.h"

namespace dmcs { namespace generator {

class RingOptTopoGenerator : public OptTopologyGenerator
{
public:
  RingOptTopoGenerator(std::size_t system_size, LocalInterfaceMapPtr lcim)
    : OptTopologyGenerator(system_size, lcim)
  { }


  void
  create_opt_interface(std::size_t id)
  {
    if (id == system_size)
      {
	return;
      }

    if (system_size > 1)
      {
	NewBeliefStatePtr bs_this_next = getInterface(lcim, id-1, id);
	NewBeliefStatePtr bs_n_1       = getInterface(lcim, system_size-1, 0);

	(*bs_this_next) = (*bs_this_next) | (*bs_n_1);
      }

    // remove connection n --> 1
    if (id == 1)
      {
	ContextPair cp(system_size-1, 0);
	lcim->erase(cp);
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_RING_OPT_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
