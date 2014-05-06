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
 * @file   testReturnPlanParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Oct  23 17:49:26 2012
 * 
 * @brief  
 * 
 * 
 */

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testReturnPlanParser"
#include <boost/test/unit_test.hpp>

#include "mcs/BeliefStateOffset.h"
#include "parser/ReturnPlanParser.h"

#include <sstream>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testReturnPlanParser )
{
  std::size_t system_size = 5;
  std::size_t bs_size = 10;
  
  BeliefStateOffset* bso_instance = BeliefStateOffset::create(system_size, bs_size);

  for (std::size_t i = 0; i <= 4; ++i)
    {
      std::stringstream out;
      out << i;

      std::string returnplan_file = "../../examples/context" + out.str() + ".rp";
      ReturnPlanMapPtr rpm = ReturnPlanParser::parseFile(returnplan_file);
      
      std::cout << "Parsing return plans for context " << i << ". Got return interfaces:" << std::endl;
      for (ReturnPlanMap::const_iterator it = rpm->begin(); it != rpm->end(); ++it)
	{
	  std::size_t parent_id = it->first;
	  NewBeliefState* interface = it->second;
	  
	  std::cout << parent_id << " " << *interface << std::endl;
	}
      std::cout << std::endl;
    }
}

// Local Variables:
// mode: C++
// End: