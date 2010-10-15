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
 * @file   DynCtxGenGridBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jul   26 10:32:27 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef DYNAMIC_CONTEXT_GENERATOR_GRID_BUILDER
#define DYNAMIC_CONTEXT_GENERATOR_GRID_BUILDER

#include "DynCtxGenBaseBuilder.h"

namespace dmcs {

class DynCtxGenGridBuilder : public DynCtxGenBaseBuilder
{
public:
  DynCtxGenGridBuilder(MatchVecPtr mt_, GraphPtr initial_topology_, 
		       BridgeRuleVecVecPtr bridge_rules_,
		       BridgeRuleVecPtr pattern_bridge_rules_, 
		       IntVecPtr no_sbridge_atoms_,
		       std::size_t m_, std::size_t n_)
    : DynCtxGenBaseBuilder(mt_, initial_topology_, bridge_rules_, 
			   pattern_bridge_rules_, no_sbridge_atoms_, m_*n_),
      m(m_), n(n_)
  { }

  //  1          ---> 2           ---> 3           ---> ... ---> n
  //  |               |                |                         |
  //  |               |                |                         |
  // n+1        ---> n+2         ---> n+3         ---> ... ---> 2n
  //  |               |                |                         |
  //  |               |                |                         |
  // ...
  //  |               |                |                         |
  //  |               |                |                         |
  // (m-1)*n + 1 --> (m-1)*n + 2 ---> (m-1)*n + 3 ---> ... ---> m*n

  void
  generate_dynamic_system()
  {
    std::size_t id;
    for (std::size_t i = 0; i < m-1; ++i)
      {
	for (std::size_t j = 1; j < n; ++j)
	  {
	    id = i*n + j;
	    generate_matches(id, id+1);
	    generate_matches(id, id+n);
	    generate_bridge_rules(id);
	  }
	id = (i+1)*n;
	generate_matches(id, id+n);
	generate_bridge_rules(id);
      }

    for (std::size_t j = 1; j < n; ++j)
      {
	id = (m-1)*n + j;
	generate_matches(id, id+1);
	generate_bridge_rules(id);
      }
  }

private:
  std::size_t m;
  std::size_t n;

};

} // namespace dmcs

#endif

// Local Variables:
// mode: C++
// End:
