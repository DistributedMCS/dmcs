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

// Local Variables:
// mode: C++
// End:
