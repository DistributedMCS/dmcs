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

#include "mcs/QueryPlan.h"
#include "mcs/NewNeighbor.h"
#include "parser/Parser.hpp"

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testBridgeRuleParser )
{
  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string queryplan_file(ex);
  queryplan_file += "/bridgeRuleParserTest-queryplan.txt";
  QueryPlanParser_t queryplan_parser;
  ContextQueryPlanMapPtr qp1 = queryplan_parser.parseFile(queryplan_file);

  std::string bridge_rules_file(ex);
  bridge_rules_file += "/bridgeRuleParserTest-rules.txt";

  const std::size_t ctx_id = 1;
  ContextQueryPlanMapPtr_CtxID valuesHolder(qp1, ctx_id);
  BridgeRuleParser_t bridgerule_parser(valuesHolder);
  BridgeRuleParserReturnVal ret_val = bridgerule_parser.parseFile(bridge_rules_file);

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
