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
 * @file   DynSocialTopoGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jul   6 15:17:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DYN_SOCIAL_TOPO_GENERATOR
#define DYN_SOCIAL_TOPO_GENERATOR

#include "generator/DynTopologyGenerator.h"

#define GROUP_SIZE 5

namespace dmcs { namespace generator {

class DynSocialTopoGenerator : public DynTopologyGenerator
{
public:
  DynSocialTopoGenerator(MatchVecPtr mt_, GraphPtr initial_topology_, 
			 BridgeRuleVecVecPtr bridge_rules_,
			 BridgeRuleVecPtr pattern_bridge_rules_, 
			 IntVecPtr no_sbridge_atoms_,
			 std::size_t poolsize_)
    : DynTopologyGenerator(mt_, initial_topology_, bridge_rules_, 
			   pattern_bridge_rules_, no_sbridge_atoms_, 
			   poolsize_)
  { }
  
  void
  generate_dynamic_system()
  {
    std::size_t start = 1;
    std::size_t end;

    std::vector<bool> has_matches;
    has_matches.assign(poolsize, false);

    while (start < poolsize)
      {
	// choose a number of consecutive contexts to group together
	end = poolsize < start + GROUP_SIZE ? poolsize + 1 : start + GROUP_SIZE;
	for (std::size_t i = start; i < end-1; ++i)
	  {
	    for (std::size_t j = i+1; j < end; ++j)
	      {
		// for each pair of contexts in the group, the
		// probability for a connection from one to another is
		// 0.6
		std::size_t p = rand() % 10;
		if (p < 6)
		  {
		    generate_matches(i, j);
		    has_matches[i] = true;
		  }

		p = rand() % 10;
		if (p < 6)
		  {
		    generate_matches(j, i);
		    has_matches[j] = true;
		  }
	      }
	  }

	// from each group, there is one edge to the starting point of the next group
	if (end <= poolsize)
	  {
	    generate_matches(end-1, end);
	    has_matches[end-1] = true;
	  }

	// continue to the next group
	start = end;
      }
    for (std::size_t i = 1; i <= poolsize; ++i)
      {
	if (has_matches[i])
	  {
	    generate_bridge_rules(i);
	  }
      }
  }
};

  } // namespace generator
} // namespace dmcs

#endif // DYN_SOCIAL_TOPO_GENERATOR

// Local Variables:
// mode: C++
// End:
