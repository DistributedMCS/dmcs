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
#include "mcs/BeliefStateOffset.h"
#include "mcs/BeliefTable.h"
#include "mcs/BridgeRuleEvaluator.h"
#include "mcs/Heads.h"

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

  std::size_t NO_BS = 3;
  std::size_t BS_SIZE = 20;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BS, BS_SIZE);
  
  NewBeliefState* input = new NewBeliefState(NO_BS, BS_SIZE);

  std::vector<std::size_t>& starting_offsets = bso->getStartingOffsets();

  // setting up input: (\epsilon {d2, e2}, {g3, i3})
  input->set(1, idd2.address, starting_offsets);
  input->set(1, ide2.address, starting_offsets);
  input->set(2, idg3.address, starting_offsets);
  input->set(2, idh3.address, starting_offsets);

  Heads* heads = evaluate(brtab, input, starting_offsets);
  HeadsPlusBeliefState* headsbs = static_cast<HeadsPlusBeliefState*>(heads);
  
  NewBeliefState::TruthVal a1_val = headsbs->getHeads()->test(0, ida1.address, starting_offsets);
  NewBeliefState::TruthVal b1_val = headsbs->getHeads()->test(0, idb1.address, starting_offsets);
  NewBeliefState::TruthVal c1_val = headsbs->getHeads()->test(0, idc1.address, starting_offsets);

  BOOST_CHECK_EQUAL(a1_val, NewBeliefState::DMCS_UNDEF);
  BOOST_CHECK_EQUAL(b1_val, NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(c1_val, NewBeliefState::DMCS_TRUE);
}

// Local Variables:
// mode: C++
// End:
