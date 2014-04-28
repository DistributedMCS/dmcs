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

#include "parser/DLVResultParser.h"
#includee "SimpleBeliefStateAdder.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "test"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


BOOST_AUTO_TEST_CASE ( testDLVResultParser )
{
  // some answer sets
  std::vector<std::string> ans = {"{arc(a,b), arc(b,c), arc(b,d)}",
				  "{arc(a,b), arc(b,c), -arc(b,d)}",
				  "{arc(a,b), -arc(b,c), arc(b,d)}"};

  std::string all_answers = "";
  for (std::vector<std::string>::const_iterator it = ans.begin(); it != ans.end(); ++it)
    {
      all_answers = all_answers + (*it) + "\n";
    }

  std::istringstream iss(all_answers);

  BeliefTablePtr btab(new BeliefTable);
  Belief belief_eps1(1, "epsilon");
  ID id_eps1 = btab.storeAndGetID(belief_eps1);

  DLVResultParser dlv_parser(1, btab);

  SimpleBeliefStateAdder adder;

  dlv_parser.parse(iss, adder);

  for (std::vector<NewBeliefState*>::const_iterator it = adder.begin();
       it != adder.end(); ++it)
    {
      sdt::cerr << **it << std::endl;
    }

  std::cerr << "BeliefTable: " << btab << std::endl;
}




// Local Variables:
// mode: C++
// End:
