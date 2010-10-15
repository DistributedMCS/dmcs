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
 * @file   DynCtxGenRandomBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jul   6 15:17:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DYNAMIC_CONTEXT_GENERATOR_RANDOM_BUILDER
#define DYNAMIC_CONTEXT_GENERATOR_RANDOM_BUILDER

#include "DynCtxGenBaseBuilder.h"

namespace dmcs {

class DynCtxGenRandomBuilder : public DynCtxGenBaseBuilder
{
public:
  DynCtxGenRandomBuilder(MatchVecPtr mt_, GraphPtr initial_topology_, 
			 BridgeRuleVecVecPtr bridge_rules_,
			 BridgeRuleVecPtr pattern_bridge_rules_, 
			 IntVecPtr no_sbridge_atoms_,
			 std::size_t density_,
			 std::size_t poolsize_)
    : DynCtxGenBaseBuilder(mt_, initial_topology_, bridge_rules_, 
			   pattern_bridge_rules_, no_sbridge_atoms_, 
			   poolsize_),
      density(density_)
  { }

  void
  generate_dynamic_system()
  {
    for (std::size_t i = 1; i <= poolsize; ++i)
      {
	for (std::size_t j = 1; j <= poolsize; ++j)
	  {
	    bool has_matches = false;

	    if (i != j)
	      {
		// a probability of 7/10 that there is an edge from i
		// to j

		std::size_t flipping = (rand() % 10);
		if (flipping < density)
		  {
		    generate_matches(i, j);
		    has_matches = true;
		  }
	      }

	    if (has_matches)
	      {
		
		generate_bridge_rules(i);
	      }
	  }
      }
  }

private:
  std::size_t density;
};


} // namespace dmcs

#endif // DYNAMIC_CONTEXT_GENERATOR_RANDOM_BUILDER

// Local Variables:
// mode: C++
// End:
