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
 * @file   testResultParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  13 9:00:00 2012
 * 
 * @brief  
 * 
 * 
 */

#include <bm/bm.h>

#include "mcs/BeliefStateOffset.h"
#include "parser/DLVResultParser.h"
#include "SimpleBeliefStateAdder.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test"
#include <boost/test/unit_test.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


BOOST_AUTO_TEST_CASE ( testDLVResultParser )
{
  std::size_t NO_BLOCKS = 3;
  std::size_t SIZE_BS = 5;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BLOCKS, SIZE_BS);

  // some answer sets
  std::string ans1 = "{-arc(a,b), arc(b,c), arc(b,d)}";
  std::string ans2 = "{arc1, arc(b,c), -arc(b,d)}";
  std::string ans3 = "{arc(a,b), -arc(b,c), arc(b,d)}";
  std::vector<std::string> ans;
  ans.push_back(ans1);
  ans.push_back(ans2);
  ans.push_back(ans3);

  BeliefTablePtr btab(new BeliefTable);
  Belief belief_eps1(1, "epsilon");
  ID id_eps1 = btab->storeAndGetID(belief_eps1);

  std::string all_answers = "";
  for (std::vector<std::string>::const_iterator it = ans.begin(); it != ans.end(); ++it)
    {
      all_answers = all_answers + (*it) + "\n";
    }

  std::istringstream iss(all_answers);

  std::size_t ctx_id = 0;
  DLVResultParser dlv_parser(ctx_id, btab);

  std::vector<NewBeliefState*> results;
  SimpleBeliefStateAdder adder(results);

  dlv_parser.parse(iss, adder);

  for (std::vector<NewBeliefState*>::const_iterator it = results.begin();
       it != results.end(); ++it)
    {
      std::cerr << **it << std::endl;
    }

  std::cerr << "BeliefTable: " << std::endl << *btab << std::endl;

  BOOST_CHECK_EQUAL(results.size(), 3);
  BOOST_CHECK_EQUAL(btab->getSize(), 5);
}




// Local Variables:
// mode: C++
// End:
