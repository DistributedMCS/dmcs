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
 * @file   testBridgeRuleParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Sep  17 15:50:01 2012
 * 
 * @brief  
 * 
 * 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBridgeRuleParser"
#include <boost/test/unit_test.hpp>

#include "parser/BridgeRuleParser.h"
#include "mcs/QueryPlan.h"
#include "mcs/NewNeighbor.h"
#include "parser/QueryPlanParser.h"

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testBridgeRuleParser )
{
  std::string queryplan_file = "../../examples/context1.qp";
  ContextQueryPlanMapPtr qp1 = QueryPlanParser::parseFile(queryplan_file);

  std::string bridge_rules_file = "../../examples/bridge_rules.inp";
  const unsigned int ctx_id = 1;
  BridgeRuleParserReturnVal ret_val = BridgeRuleParser::parseFile(bridge_rules_file, qp1, ctx_id);
  BridgeRuleTablePtr brtab = ret_val.first;
  NewNeighborVecPtr neighbors = ret_val.second;

  std::cout << "Got bridge rules:" << std::endl;
  std::pair<BridgeRuleTable::AddressIterator, BridgeRuleTable::AddressIterator> iters = brtab->getAllByAddress();
  for (BridgeRuleTable::AddressIterator it = iters.first; it != iters.second; ++it)
    {
      const BridgeRule& r = *it;
      std::cout << r << std::endl;
    }

  std::cout << "Got neighbors:" << std::endl;
  for (NewNeighborVec::const_iterator it = neighbors->begin(); it != neighbors->end(); ++it)
    {
      const NewNeighbor& n = **it;
      std::cout << n << std::endl;
    }
}

// Local Variables:
// mode: C++
// End:
