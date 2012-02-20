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

#include "mcs/BeliefStateOffset.h"
#include "dmcs/DLVEngine.h"
#include "dmcs/DLVInstantiator.h"
#include "dmcs/DLVEvaluator.h"
#include "network/NewConcurrentMessageDispatcher.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testAbstractContext"
#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

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
  EvaluatorPtr dlv_eval = dlv_inst->createEvaluator(dlv_inst_wp);

  BOOST_CHECK_EQUAL(dlv_engine->getNoInstantiators(), 1);
  BOOST_CHECK_EQUAL(dlv_inst->getNoEvaluators(), 1);

  EvaluatorWPtr dlv_eval_wp(dlv_eval);
  dlv_inst->removeEvaluator(dlv_eval_wp);
  BOOST_CHECK_EQUAL(dlv_inst->getNoEvaluators(), 0);
}


BOOST_AUTO_TEST_CASE ( testRunningDLV )
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

  std::size_t no_bs = 4;
  std::size_t bs_size = 10;

  BeliefStateOffset* bso = BeliefStateOffset::create(no_bs, bs_size);

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(queue_size, no_neighbors));
  EvaluatorPtr dlv_eval = dlv_inst->createEvaluator(dlv_inst_wp);

  std::size_t ctx_id = 0;

  Belief epsilon(ctx_id, "epsilon0");
  Belief bird_tweety(ctx_id, "bird(tweety)");
  Belief flies_tweety(ctx_id, "flies(tweety)");
  Belief not_flies_tweety(ctx_id, "not_flies(tweety)");
  Belief fit_tweety(ctx_id, "fit(tweety)");

  BeliefTablePtr btab(new BeliefTable);
  ID id_epsilon = btab->storeAndGetID(epsilon);
  ID id_bird_tweety = btab->storeAndGetID(bird_tweety);
  ID id_flies_tweety = btab->storeAndGetID(flies_tweety);
  ID id_not_files_tweety = btab->storeAndGetID(not_flies_tweety);
  ID id_fit_tweety = btab->storeAndGetID(fit_tweety);

  DLVEvaluatorPtr dlv_eval_casted = boost::static_pointer_cast<DLVEvaluator>(dlv_eval);
  
  boost::thread evaluation_thread(*dlv_eval_casted, ctx_id, btab, md);

  boost::posix_time::milliseconds n(1000);
  boost::this_thread::sleep(n);

  NewBeliefState* head_input = new NewBeliefState(no_bs, bs_size);
  const NewBeliefState* input_bs = new NewBeliefState(no_bs, bs_size);
  head_input->set(ctx_id, id_fit_tweety.address, bso->getStartingOffsets());

  HeadsPlusBeliefState* heads = new HeadsPlusBeliefState(head_input, input_bs);

  
  NewBeliefState* end_head_input = NULL;
  NewBeliefState* end_input_bs = NULL;
  HeadsPlusBeliefState* end_heads = new HeadsPlusBeliefState(end_head_input, end_input_bs);

  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, dlv_eval->getInQueue(), heads, timeout);
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, dlv_eval->getInQueue(), end_heads, timeout);

  evaluation_thread.join();

  // input program is:
  //
  // flies(X) v not_flies(X) :- bird(X), fit(X).
  // bird(tweety).
  //
  // with heads = {fit(tweety).}
  //
  // expect 2 answers [1, 2, 4] and [1, 3, 4]
  
  std::size_t count = 0;
  do
    {
      int timeout = 0;
      HeadsBeliefStatePair* ans = md->receive<HeadsBeliefStatePair>(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, dlv_eval->getOutQueue(), timeout);
      if (ans->second == NULL)
	{
	  break;
	}
      ++count;
    }
  while (1);
  
  BOOST_CHECK_EQUAL(count, 2);
}

// Local Variables:
// mode: C++
// End:
