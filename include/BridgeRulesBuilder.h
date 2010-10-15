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
 * @file   BridgeRulesBuilder.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Sat Jan  02 16:48:24 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef BRIDGE_RULES_BUILDER_H
#define BRIDGE_RULES_BUILDER_H

#include <map>

#include "BaseBuilder.h"
#include "Neighbor.h"
#include "Rule.h"
#include "Theory.h"
#include "Signature.h"
#include "Variable.h"

namespace dmcs {

template<class Grammar>
class BridgeRulesBuilder : public BaseBuilder<Grammar>
{
public:
  // with ctx_id_ present, we can remove sig_ and take global_sigs[ctx_id-1]
  BridgeRulesBuilder(std::size_t ctx_id, 
		     BridgeRulesPtr& bridge_rules_, 
		     NeighborListPtr& neighbor_list_,
		     SignatureVecPtr& global_sigs_);

  void
  buildNode (typename BaseBuilder<Grammar>::node_t& node);

private:
  void
  build_disjunctive_head(typename BaseBuilder<Grammar>::node_t& node, BridgeRulePtr& r);

  void
  build_body(typename BaseBuilder<Grammar>::node_t& node, BridgeRulePtr& r);

  BridgeAtom
  build_bridge_atom(typename BaseBuilder<Grammar>::node_t& node);

private:
  std::size_t ctx_id; 
  std::size_t ctx_var_counter;
  std::map<std::string, ContextTerm> vm;
  BridgeRulesPtr& bridge_rules;
  NeighborListPtr& neighbor_list;
  SignatureVecPtr& global_sigs;
};

} // namespace dmcs

#endif // BRIDGE_RULES_BUILDER_H

#include "BridgeRulesBuilder.tcc"

// Local Variables:
// mode: C++
// End:
