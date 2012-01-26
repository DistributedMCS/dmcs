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
 * @file   testAbstractContext.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  26 16:50:01 2012
 * 
 * @brief  
 * 
 * 
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/DLVEngine.h"
#include "dmcs/DLVInstantiator.h"
#include "dmcs/DLVEvaluator.h"
#include "network/NewConcurrentMessageDispatcher.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testAbstractContext"
#include <boost/test/unit_test.hpp>

#include <iostream>
#include <string>
#include <assert.h>

using namespace dmcs;

// run me with
// EXAMPLESDIR=../../examples ./testAbstractContext
BOOST_AUTO_TEST_CASE ( testEngineInstantiatorEvaluatorCreation )
{
  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  const char* ex = getenv("EXAMPLESDIR");
  assert (ex != 0);
  std::string kbspec(ex);
  kbspec += "/testAbstractContext.inp";

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);
  InstantiatorWPtr dlv_inst_wp(dlv_inst);

  std::size_t queue_size = 5;
  std::size_t no_neighbors = 3;
  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(queue_size, no_neighbors));
  EvaluatorPtr dlv_eval = dlv_inst->createEvaluator(dlv_inst_wp, md);

  BOOST_CHECK_EQUAL(dlv_engine->getNoInstantiators(), 1);
  BOOST_CHECK_EQUAL(dlv_inst->getNoEvaluators(), 1);

  EvaluatorWPtr dlv_eval_wp(dlv_eval);
  dlv_inst->removeEvaluator(dlv_eval_wp);
  BOOST_CHECK_EQUAL(dlv_inst->getNoEvaluators(), 0);
}

// Local Variables:
// mode: C++
// End:
