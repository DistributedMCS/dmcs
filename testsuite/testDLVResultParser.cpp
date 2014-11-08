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
 * @file   testDLVResultParser.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  13 9:00:00 2012
 * 
 * @brief  
 * 
 * 
 */

#include <bm/bm.h>

#include "parser/Parser.hpp"
#include "mcs/BeliefTable.h"

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
  BeliefStateOffset *bso_instance = BeliefStateOffset::create(10, 5);

  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);

  std::string filename_QueryPlan(ex); 
  std::string filename_DLVResult(ex); 

  filename_QueryPlan += "/dlvResultParserTest-queryplan.txt";
  filename_DLVResult += "/dlvResultParserTest-output.txt";

  QueryPlanParser_t queryplan_parser;
  ContextQueryPlanMapPtr plan = queryplan_parser.parseFile(filename_QueryPlan);

  std::size_t ctxID = 1;
  ContextQueryPlanMap::const_iterator it = plan->find(ctxID);
  assert (it != plan->end());
  
  const ContextQueryPlan &cqp = it->second;
  const BeliefTablePtr localSignature = cqp.localSignature;

  const BeliefTablePtr_CtxID valuesHolder(localSignature, ctxID);

  std::cout << *localSignature << std::endl << std::endl;

  DLVResultParser_t dlvResult_parser(valuesHolder);
  NewBeliefState *bs = dlvResult_parser.parseFile(filename_DLVResult);
  
  std::cout << *bs << std::endl;
}

// Local Variables:
// mode: C++
// End:
