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
 * @file   BridgeRuleParser.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  17 15:59:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef __BRIDGE_RULE_PARSER_H__
#define __BRIDGE_RULE_PARSER_H__

#include "mcs/Rule.h"
#include "QueryPlan.h"

namespace dmcs {

class BridgeRuleParser
{
public:
  //static BridgeRuleListPtr parseStream(std::istream& in);
  //static BridgeRuleListPtr parseFile(const std::string& infile);
  //static BridgeRuleListPtr parseString(const std::string& instr);
  static bool parseString(const std::string& instr,
			  ContextQueryPlanMapPtr& queryplan);
};

} // namespace dmcs

#endif // __BRIDGE_RULE_PARSER_H__


// Local Variables:
// mode: C++
// End:
