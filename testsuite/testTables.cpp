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
 * @file   testTables.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  12 11:03:46 2011
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BeliefTable.h"
#include "mcs/BridgeRuleTable.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testTables"
#include <boost/test/unit_test.hpp>

#include <iostream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testBeliefTable )
{
  uint16_t ctx2 = 2;
  uint16_t ctx3 = 3;

  Belief belief_a(ctx2, "a");
  std::string stra("a");

  Belief belief_b(ctx2, "b");

  Belief belief_c(ctx3, "c");
  std::string strc("c");

  Belief belief_d(ctx3, "d");

  {
    BeliefTable btab;
    BOOST_CHECK_EQUAL(ID_FAIL, btab.getIDByString(stra));
    
    ID ida = btab.storeAndGetID(belief_a);
    BOOST_CHECK_EQUAL(sizeof(ida), 8);
    BOOST_CHECK_EQUAL(ida, btab.getIDByString(stra));
    BOOST_CHECK_EQUAL(ida.address, 0);
    
    LOG(INFO, "BeliefTable" << btab);
  }

  {
    BeliefTable btab;
    ID ida = btab.storeAndGetID(belief_a);
    ID idb = btab.storeAndGetID(belief_b);
    ID idc = btab.storeAndGetID(belief_c);
    ID idd = btab.storeAndGetID(belief_d);

    BOOST_CHECK_EQUAL(ida.address, 0);
    BOOST_CHECK_EQUAL(idb.address, 1);
    BOOST_CHECK_EQUAL(idc.address, 2);
    BOOST_CHECK_EQUAL(idd.address, 3);

    BOOST_CHECK_EQUAL(ida.kind, belief_a.kind);
    BOOST_CHECK_EQUAL(idb.kind, belief_b.kind);
    BOOST_CHECK_EQUAL(idc.kind, belief_c.kind);
    BOOST_CHECK_EQUAL(idd.kind, belief_d.kind);

    ID getida = btab.getIDByString(stra);
    BOOST_CHECK_EQUAL(getida.kind, belief_a.kind);
    BOOST_CHECK_EQUAL(getida.address, 0);

    ID getidc = btab.getIDByString(strc);
    BOOST_CHECK_EQUAL(getidc.kind, belief_c.kind);
    BOOST_CHECK_EQUAL(getidc.address, 2);

    LOG(INFO, "BeliefTable" << btab);
  }
}



BOOST_AUTO_TEST_CASE ( testBridgeRuleTable )
{
  uint16_t ctx1 = 1;
  uint16_t ctx2 = 2;
  uint16_t ctx3 = 3;

  Belief belief_a(ctx1, "a");
  std::string stra("a");

  Belief belief_b(ctx2, "b");

  Belief belief_c(ctx2, "c");
  std::string strc("c");

  Belief belief_d(ctx3, "d");
  Belief belief_e(ctx3, "e");
  Belief belief_f(ctx3, "f");

  BeliefTable btab;
  ID ida = btab.storeAndGetID(belief_a);
  ID idb = btab.storeAndGetID(belief_b);
  ID idc = btab.storeAndGetID(belief_c);
  ID idd = btab.storeAndGetID(belief_d);
  ID ide = btab.storeAndGetID(belief_e);
  ID idf = btab.storeAndGetID(belief_f);

  ID id_neg_c = ID::nafLiteralFromBelief(idc); 
  ID id_neg_d = ID::nafLiteralFromBelief(idd);

  Tuple body1;
  body1.push_back(idb);
  body1.push_back(id_neg_d);

  Tuple body2;
  body2.push_back(id_neg_c);
  body2.push_back(ide);
  body2.push_back(idf);

  // a :- (2:b), not (3:d).
  BridgeRule br1(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, ida, body1);

  // a :- (3:e), (3:f), not (2:c).
  BridgeRule br2(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, ida, body2);

  BridgeRuleTable rtab;
  ID id_br1 = rtab.storeAndGetID(br1);
  BOOST_CHECK_EQUAL(id_br1.kind, rtab.getByID(id_br1).kind);
  BOOST_CHECK_EQUAL(id_br1.address, 0);

  ID id_br2 = rtab.storeAndGetID(br2);
  BOOST_CHECK_EQUAL(id_br2.address, 1);
  BOOST_CHECK(rtab.getByID(id_br2).head == ida);
  BOOST_CHECK(rtab.getByID(id_br2).body == body2);

  LOG(INFO,"BeliefTable" << btab);
  LOG(INFO,"RuleTable" << rtab);
}

// Local Variables:
// mode: C++
// End:
