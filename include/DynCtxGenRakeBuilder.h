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

#ifndef DYNAMIC_CONTEXT_GENERATOR_RAKE_BUILDER
#define DYNAMIC_CONTEXT_GENERATOR_RAKE_BUILDER

#include "DynCtxGenBaseBuilder.h"

namespace dmcs {

class DynCtxGenRakeBuilder : public DynCtxGenBaseBuilder
{
public:
  DynCtxGenRakeBuilder(MatchVecPtr mt_, GraphPtr initial_topology_, 
		       BridgeRuleVecVecPtr bridge_rules_,
		       BridgeRuleVecPtr pattern_bridge_rules_, 
		       IntVecPtr no_sbridge_atoms_,
		       std::size_t poolsize_)
    : DynCtxGenBaseBuilder(mt_, initial_topology_, bridge_rules_, 
			   pattern_bridge_rules_, no_sbridge_atoms_, 
			   poolsize_)
  { }

  // only applies for the RAKE topology
  bool
  generate_rake(std::size_t id)
  {
    bool has_matches = false;

    // a context has matches to contexts with higher id with probability of 9/10
    for (std::size_t j = id+1; j <= poolsize; ++j)
      {
	std::size_t flipping = rand() % 10;
	if (flipping < 9)
	  {
	    generate_matches(id, j);
	    generate_bridge_rules(id);
	    has_matches = true;
	}
      }
    
    // a context with id > 1 has matches to a context with lower id with probability of 2/10
    if (id > 1)
      {
	std::size_t flipping = rand() % 10;
	if (flipping < 2)
	  {
	    std::size_t pn = (rand() % (id-1)) + 1;
	    generate_matches(id, pn);
	    generate_bridge_rules(id);
	    has_matches = true;
	  }
    }
    
    return has_matches;
  }

  void
  generate_dynamic_system()
  {

    for (std::size_t i = 1; i <= poolsize; ++i)
      {
	// generate matches for context i. We requires that there must
	// be some matches for every context, except for the last one.

	bool ok;
	do
	  {
	    ok = generate_rake(i);
	    if (i == poolsize)
	    {
	      ok = true;
	    }
	  }
	while (!ok);
      }
  }

};

}

#endif // DYNAMIC_CONTEXT_GENERATOR_RAKE_BUILDER
