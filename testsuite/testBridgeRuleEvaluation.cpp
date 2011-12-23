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
 * @file   testBridgeRuleEvaluation.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  12 17:45:20 2011
 * 
 * @brief  
 * 
 * 
 */

#include <iostream>
#include "mcs/BeliefTable.h"
#include "mcs/BridgeRuleEvaluator.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBridgeRuleEvaluation"
#include <boost/test/unit_test.hpp>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testBridgeRuleEvaluation )
{
  // bridge rules:
  // a1 :- (2:d2), not (3:g3).
  // b1 :- (2:e2), (3:h3).
  // c1 :- not (2:f2), not (3:i3).

  // IMPORTANT: 
  // + ctx_id starting from 0
  // + in BeliefTable, always add an epsilon at the beginning (index 0).

  uint16_t ctx1 = 0;
  uint16_t ctx2 = 1;
  uint16_t ctx3 = 2;

  Belief belief_a1(ctx1, "a1");
  Belief belief_b1(ctx1, "b1");
  Belief belief_c1(ctx1, "c1");
  Belief belief_eps1(ctx1, "epsilon");

  Belief belief_d2(ctx2, "d2");
  Belief belief_e2(ctx2, "e2");
  Belief belief_f2(ctx2, "f2");
  Belief belief_eps2(ctx2, "epsilon");

  Belief belief_g3(ctx3, "g3");
  Belief belief_h3(ctx3, "h3");
  Belief belief_i3(ctx3, "i3");
  Belief belief_eps3(ctx3, "epsilon");

  BeliefTable btab1;
  ID id_eps1 = btab1.storeAndGetID(belief_eps1);
  ID ida1 = btab1.storeAndGetID(belief_a1);
  ID idb1 = btab1.storeAndGetID(belief_b1);
  ID idc1 = btab1.storeAndGetID(belief_c1);

  BeliefTable btab2;
  ID id_eps2 = btab2.storeAndGetID(belief_eps2);
  ID idd2 = btab2.storeAndGetID(belief_d2);
  ID ide2 = btab2.storeAndGetID(belief_e2);
  ID idf2 = btab2.storeAndGetID(belief_f2);
  ID id_neg_f2 = ID::nafLiteralFromBelief(idf2);

  BeliefTable btab3;
  ID id_eps3 = btab3.storeAndGetID(belief_eps3);
  ID idg3 = btab3.storeAndGetID(belief_g3);
  ID idh3 = btab3.storeAndGetID(belief_h3);
  ID idi3 = btab3.storeAndGetID(belief_i3);
  ID id_neg_g3 = ID::nafLiteralFromBelief(idg3);
  ID id_neg_i3 = ID::nafLiteralFromBelief(idi3);

  Tuple body1;
  body1.push_back(idd2);
  body1.push_back(id_neg_g3);

  Tuple body2;
  body2.push_back(ide2);
  body2.push_back(idh3);

  Tuple body3;
  body3.push_back(id_neg_f2);
  body3.push_back(id_neg_i3);


  BridgeRule br1(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, ida1, body1);

  BridgeRule br2(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, idb1, body2);

  BridgeRule br3(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, idc1, body3);

  TuplePtr bridge_rules(new Tuple);
  BridgeRuleTablePtr brtab(new BridgeRuleTable);

  ID id_br1 = brtab->storeAndGetID(br1);
  ID id_br2 = brtab->storeAndGetID(br2);
  ID id_br3 = brtab->storeAndGetID(br3);

  bridge_rules->push_back(id_br1);
  bridge_rules->push_back(id_br2);
  bridge_rules->push_back(id_br3);

  // prepare offset: [0, 20, 40]
  std::vector<std::size_t> starting_offset(3,0);
  for (std::size_t i = 1; i < 3; ++i)
    {
      starting_offset[i] = starting_offset[i-1] + 20;
    }

  NewBeliefState input(3);
  NewBeliefState heads(3);

  // setting up input: (\epsilon {d2, e2}, {g3, i3})
  std::size_t global_address_a1 = starting_offset[0] + ida1.address;
  std::size_t global_address_b1 = starting_offset[0] + idb1.address;
  std::size_t global_address_c1 = starting_offset[0] + idc1.address;
  std::size_t global_address_d2 = starting_offset[1] + idd2.address;
  std::size_t global_address_e2 = starting_offset[1] + ide2.address;
  std::size_t global_address_g3 = starting_offset[2] + idg3.address;
  std::size_t global_address_h3 = starting_offset[2] + idh3.address;

  set(input, global_address_d2);
  set(input, global_address_e2);
  set(input, global_address_g3);
  set(input, global_address_h3);

  evaluate(brtab, bridge_rules, input, starting_offset, heads);
  
  NewBeliefState::TruthVal a1_val = test(heads, global_address_a1);
  NewBeliefState::TruthVal b1_val = test(heads, global_address_b1);
  NewBeliefState::TruthVal c1_val = test(heads, global_address_c1);

  BOOST_CHECK_EQUAL(a1_val, NewBeliefState::DMCS_UNDEF);
  BOOST_CHECK_EQUAL(b1_val, NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(c1_val, NewBeliefState::DMCS_TRUE);
}

// Local Variables:
// mode: C++
// End:
