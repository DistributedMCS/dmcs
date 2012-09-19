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

#include "BridgeRuleParser.h"
#include "QueryPlan.h"
#include "QueryPlanParser.h"

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testBridgeRuleParser )
{
  const char* queryPlan1 =
  "["
  "  {"
  "    ContextId: 1,"
  "    LocalSignature:" 
  "    {" 
  "      1: [a, c1]," 
  "      2: [a, c2]," 
  "      3: [a, c3]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 3,"
  "    InputSignature:" 
  "    {" 
  "      1: [c, c1, c4]," 
  "      2: [c, c2, c5]," 
  "      3: [c, c3, c6]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 4,"
  "    InputSignature:" 
  "    {" 
  "      4: [dprime, c1, c4]," 
  "      5: [dprime, c2, c5]," 
  "      6: [dprime, c3, c6]," 
  "    }," 
  "  }," 
  "]";

  ContextQueryPlanMapPtr qp1 = QueryPlanParser::parseString(queryPlan1);

  const char* brrule = "a(c1) :- not (1:a(c1)), (4:dprime(c1,c4)). a(c2) :- not (1:a(c1)), (4:dprime(c1,c4)).";
  const unsigned int ctx_id = 1;
  bool r = BridgeRuleParser::parseString(brrule, qp1, ctx_id);
}
