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

#include "Rule.h"
#include "generator/dmcsGen.h"
#include "generator/TopologyGenerator.h"

namespace dmcs { namespace generator {

typedef std::vector<std::size_t> Interface;
typedef std::vector<Interface> InterfaceVec;
typedef boost::shared_ptr<InterfaceVec> InterfaceVecPtr;

class ContextGenerator
{
public:
  ContextGenerator(NeighborVecListPtr orig_topo_, NeighborVecListPtr opt_topo_,
		   InterfaceVecPtr context_interfaces_, std::size_t no_atoms_,
		   std::size_t no_bridge_rules_, std::string& prefix_)
    : orig_topo(orig_topo_), opt_topo(opt_topo_), 
      context_interfaces(context_interfaces_),
      no_atoms(no_atoms_), no_bridge_rules(no_bridge_rules_),
      prefix(prefix_)
  {
    RulesPtr rs(new Rules);
    BridgeRulesPtr brs(new BridgeRules);

    local_kb = rs;
    bridge_rules = brs;
  }

  void
  generate();

  void
  generate_local_kb(std::size_t id);

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

protected:
  RulesPtr local_kb;
  BridgeRulesPtr bridge_rules;

  NeighborVecListPtr orig_topo;
  NeighborVecListPtr opt_topo;
  InterfaceVecPtr context_interfaces;

  std::size_t no_atoms;
  std::size_t no_bridge_rules;

  std::string& prefix;
};

  } // namespace generator
} // namespace dmcs

#endif // GEN_CONTEXT_GENERATOR_H

// Local Variables:
// mode: C++
// End:
