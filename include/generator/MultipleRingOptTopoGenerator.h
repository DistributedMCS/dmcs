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
 * @file   MultipleRingOptTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  20 15:26:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_MULTIPLE_RING_OPT_TOPO_GENERATOR_H
#define GEN_MULTIPLE_RING_OPT_TOPO_GENERATOR_H

#include "generator/OptTopologyGenerator.h"

namespace dmcs { namespace generator {

class MultipleRingOptTopoGenerator : public OptTopologyGenerator
{
public:
  MultipleRingOptTopoGenerator(std::size_t system_size, LocalInterfaceMapPtr lcim)
    : OptTopologyGenerator(system_size, lcim)
  { }

  void
  create_opt_interface(std::size_t id)
  {
    const std::size_t earLoopSize = (system_size + 3) / 3;
    const std::size_t secondLoopJump = earLoopSize - 1;
    const std::size_t firstLoopEnd = 2 + earLoopSize + earLoopSize -3;

    if ( (id == firstLoopEnd) || ( id == system_size))
      {
	return;
      }
    NewBeliefStatePtr bs_2a = getInterface(lcim, firstLoopEnd-1, 2);
    NewBeliefStatePtr bs_2b = getInterface(lcim, earLoopSize, 0);
    NewBeliefStatePtr bs_2c = getInterface(lcim, system_size-1, earLoopSize-1);

    if (id == earLoopSize +1)
      {
	NewBeliefStatePtr bs_11 = getInterface(lcim, id-1, id+secondLoopJump-1);

	(*bs_11) = (*bs_11) | (*bs_2a);
	(*bs_11) = (*bs_11) | (*bs_2b);
	(*bs_11) = (*bs_11) | (*bs_2c);
      }
    else
      {
	NewBeliefStatePtr bs_12 = getInterface(lcim, id-1, id);
	(*bs_12) = (*bs_12) | (*bs_2a);
	(*bs_12) = (*bs_12) | (*bs_2b);
	(*bs_12) = (*bs_12) | (*bs_2c);
      }

    if (id == 2)
      {
	NewBeliefStatePtr bs_11 = getInterface(lcim, id-1, id+earLoopSize-1);
	(*bs_11) = (*bs_11) | (*bs_2a);
	(*bs_11) = (*bs_11) | (*bs_2b);
	(*bs_11) = (*bs_11) | (*bs_2c);
      }

    if (id == 1)
      {
	ContextPair p1(firstLoopEnd-1, 2);
	ContextPair p2(earLoopSize, 0);
	ContextPair p3(system_size-1, earLoopSize-1);

	lcim->erase(p1);
	lcim->erase(p2);
	lcim->erase(p3);
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_MULTIPLE_RING_OPT_TOPO_GENERATOR_H

// Local Variables:
// mode: C++
// End:
