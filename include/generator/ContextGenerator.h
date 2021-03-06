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
 * @file   ContextGenerator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Aug  30 15:58:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef GEN_CONTEXT_GENERATOR_H
#define GEN_CONTEXT_GENERATOR_H

#define LP_EXT  ".lp"
#define BR_EXT  ".br"

#include <map>

#include "BeliefState.h"
#include "Rule.h"
#include "generator/OptTopologyGenerator.h"

namespace dmcs { namespace generator {

typedef std::vector<std::size_t> Interface;
typedef std::vector<Interface> InterfaceVec;
typedef boost::shared_ptr<InterfaceVec> InterfaceVecPtr;


class ContextGenerator
{
public:
  ContextGenerator(NeighborVec2Ptr orig_topo_,
		   InterfaceVecPtr context_interfaces_, SignatureVecPtr sigmas_,
		   BeliefStatePtr minV_, LocalInterfaceMapPtr lcim_,
		   std::size_t no_atoms_, 
		   std::size_t no_bridge_rules_, 
		   std::size_t topology_type_,
		   std::string& prefix_)
    : orig_topo(orig_topo_), context_interfaces(context_interfaces_), 
      sigmas(sigmas_), minV(minV_), lcim(lcim_), no_atoms(no_atoms_), 
      no_bridge_rules(no_bridge_rules_), 
      topology_type(topology_type_), prefix(prefix_),
      local_kb(new Rules), bridge_rules(new BridgeRules), 
      system_size(context_interfaces_->size())
  { }

  void
  generate();

  void
  generate_local_kb();

  void
  generate_bridge_rule(std::size_t id);

  void
  generate_bridge_rule_list(std::size_t id);

  void
  write_local_kb(std::size_t id);

  void
  write_bridge_rules(std::size_t id);

protected:
  int
  sign();

  bool
  cover_neighbors(std::size_t id);

  BeliefSet
  local_interface(/* std::size_t id1, */ std::size_t id2);

  void
  update_min_V();

protected:
  NeighborVec2Ptr orig_topo;
  InterfaceVecPtr context_interfaces;
  SignatureVecPtr sigmas;
  BeliefStatePtr minV;
  LocalInterfaceMapPtr lcim;

  std::size_t no_atoms;
  std::size_t no_bridge_rules;
  std::size_t topology_type;
  std::string& prefix;

  RulesPtr local_kb;
  BridgeRulesPtr bridge_rules;
  std::size_t system_size;
};


  } // namespace generator
} // namespace dmcs

#endif // GEN_CONTEXT_GENERATOR_H

// Local Variables:
// mode: C++
// End:
