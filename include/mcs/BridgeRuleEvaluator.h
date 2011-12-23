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
 * @file   BridgeRuleEvaluator.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  23 16:17:32 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BRIDGE_RULE_EVALUATOR_H
#define BRIDGE_RULE_EVALUATOR_H

#include "mcs/BridgeRuleTable.h"
#include "mcs/NewBeliefState.h"

namespace dmcs {

bool
satisfied(const BridgeRule& r, 
	  const NewBeliefState& input,
	  const std::vector<std::size_t>& starting_offset);

void evaluate(const BridgeRuleTablePtr& brtab,
	      const TuplePtr& bridge_rules,
	      const NewBeliefState& input, 
	      const std::vector<std::size_t>& starting_offset,
	      NewBeliefState& heads);

} // namespace dmcs

#endif // BRIDGE_RULE_EVALUATOR_H
