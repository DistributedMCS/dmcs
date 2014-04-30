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
 * @file   ContextGeneratorNonDeterministic.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Apr  30 04:34:20 2014
 * 
 * @brief  
 * 
 * 
 */

#include "generator/ContextGeneratorBase.h"

#ifndef __GEN_CONTEXT_GENERATOR_NON_DETERMINISTIC_H__
#define __GEN_CONTEXT_GENERATOR_NON_DETERMINISTIC_H__

namespace dmcs { namespace generator {

class ContextGeneratorNonDeterministic : public ContextGeneratorBase
{
public:
  ContextGeneratorNonDeterministic(NeighborVec2Ptr orig_topo_,
				   InterfaceVecPtr context_interfaces_, BeliefTableVecPtr sigma_vec,
				   NewBeliefStatePtr minV_, LocalInterfaceMapPtr lcim_,
				   std::size_t no_atoms_, 
				   std::size_t no_bridge_rules_, 
				   std::size_t topology_type_,
				   std::string& prefix_)
    : ContextGeneratorBase(orig_topo_, context_interfaces_, sigma_vec, minV_, lcim_,
			   no_atoms_, no_bridge_rules_, topology_type_, prefix_)
  { }

  ~ContextGeneratorNonDeterministic()
  { }

  void
  generate_local_kb();
};
    
} // namespace generator
} // namespace dmcs

#endif // __GEN_CONTEXT_GENERATOR_NON_DETERMINISTIC_H__

// Local Variables:
// mode: C++
// End:
