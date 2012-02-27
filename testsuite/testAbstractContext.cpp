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

#include "mcs/Belief.h"
#include "mcs/BeliefStateOffset.h"
#include "mcs/BridgeRuleTable.h"
#include "mcs/NewContext.h"
#include "mcs/ForwardMessage.h"
#include "mcs/NewNeighbor.h"
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
// EXAMPLESDIR=../../examples ./testAbstractContext --catch_system_errors=no


void
init_local_kb(std::size_t ctx_id,
	      std::string& kbspec,
	      BeliefTablePtr& btab)
{
  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //kbspec = ex;
  //kbspec += "/abcdContext.inp";
  kbspec = "../../examples/abcdContext.inp";

  Belief belief_epsilon1(ctx_id, "epsilon");
  Belief belief_a(ctx_id, "a");
  Belief belief_b(ctx_id, "b");
  Belief belief_c(ctx_id, "c");
  Belief belief_d(ctx_id, "d");
  Belief belief_e(ctx_id, "e");
  Belief belief_f(ctx_id, "f");
  Belief belief_g(ctx_id, "g");
  Belief belief_h(ctx_id, "h");
  Belief belief_i(ctx_id, "i");
  Belief belief_j(ctx_id, "j");

  BeliefTablePtr btab1(new BeliefTable);
  btab1->storeAndGetID(belief_epsilon1);
  btab1->storeAndGetID(belief_a);
  btab1->storeAndGetID(belief_b);
  btab1->storeAndGetID(belief_c);
  btab1->storeAndGetID(belief_d);
  btab1->storeAndGetID(belief_e);
  btab1->storeAndGetID(belief_f);
  btab1->storeAndGetID(belief_g);
  btab1->storeAndGetID(belief_h);
  btab1->storeAndGetID(belief_i);
  btab1->storeAndGetID(belief_j);
}


std::size_t
read_output_dispatcher(NewConcurrentMessageDispatcherPtr md)
{
  int timeout = 0;
  std::size_t count = 0;
  while (1)
    {
      ReturnedBeliefState* ans = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, timeout);
      if (ans->belief_state == NULL)
	{
	  break;
	}
      ++count;
      std::cerr << *(ans->belief_state) << std::endl;

    }
  std::cerr << std::endl;
  
  return count;
}



void
read_eval(NewConcurrentMessageDispatcherPtr md,
	  EvaluatorPtr dlv_eval)
{
  int timeout = 0;
  std::size_t count = 0;
  while (1)
    {
      HeadsBeliefStatePair* ans = md->receive<HeadsBeliefStatePair>(NewConcurrentMessageDispatcher::EVAL_OUT_MQ, dlv_eval->getOutQueue(), timeout);
      if (ans->second == NULL)
	{
	  break;
	}
      std::cerr << "ans = " << *(ans->second) << ", count = " << ++count << std::endl;
    }

}

BOOST_AUTO_TEST_CASE ( testDLVEvaluator )
{
  std::size_t NO_BS = 2;
  std::size_t BS_SIZE = 10;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BS, BS_SIZE);

  std::string kbspec;
  BeliefTablePtr btab(new BeliefTable);

  std::size_t QUEUE_SIZE = 100;
  std::size_t NO_NEIGHBORS = 0;

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(QUEUE_SIZE, NO_NEIGHBORS));

  std::size_t ctx_id1 = 1;
  init_local_kb(ctx_id1, kbspec, btab);

  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);
  InstantiatorWPtr dlv_inst_wp(dlv_inst);

  EvaluatorPtr dlv_eval = dlv_inst->createEvaluator(dlv_inst_wp);

  dlv_eval->init_mqs(md);

  Heads* heads1 = new Heads(NULL, 1, 3);
  Heads* heads2 = new Heads(NULL, 2, 6);
  Heads* heads3 = new Heads(NULL, 0, 0);
  Heads* heads4 = new Heads(NULL, 5, 10);

  dlv_eval->solve(ctx_id1, heads1, btab, md);
  read_eval(md, dlv_eval);
  std::cerr << std::endl;
  
  dlv_eval->solve(ctx_id1, heads2, btab, md);
  read_eval(md, dlv_eval);
  std::cerr << std::endl;

  dlv_eval->solve(ctx_id1, heads3, btab, md);
  read_eval(md, dlv_eval);
  std::cerr << std::endl;


  dlv_eval->solve(ctx_id1, heads4, btab, md);
  read_eval(md, dlv_eval);
  std::cerr << std::endl;
}


BOOST_AUTO_TEST_CASE ( testEngineInstantiatorEvaluatorCreation )
{
  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //std::string kbspec(ex);
  //kbspec += "/testAbstractContext.inp";
  std::string kbspec = "../../examples/testAbstractContext.inp";

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

  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //std::string kbspec(ex);
  //kbspec += "/testAbstractContext.inp";

  std::string kbspec = "../../examples/testAbstractContext.inp";

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);
  InstantiatorWPtr dlv_inst_wp(dlv_inst);

  std::size_t queue_size = 100;
  std::size_t no_neighbors = 3;

  std::size_t no_bs = 4;
  std::size_t bs_size = 10;

  BeliefStateOffset* bso = BeliefStateOffset::create(no_bs, bs_size);

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(queue_size, no_neighbors));
  EvaluatorPtr dlv_eval = dlv_inst->createEvaluator(dlv_inst_wp);

  dlv_eval->init_mqs(md);

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

  //  boost::posix_time::milliseconds n(1000);
  //boost::this_thread::sleep(n);

  NewBeliefState* head_input = new NewBeliefState(no_bs, bs_size);
  const NewBeliefState* input_bs = new NewBeliefState(no_bs, bs_size);
  head_input->set(ctx_id, id_fit_tweety.address, bso->getStartingOffsets());

  HeadsPlusBeliefState* heads = new HeadsPlusBeliefState(head_input, 0, 0, input_bs);
  HeadsPlusBeliefState* end_heads = NULL;
 
  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, dlv_eval->getInQueue(), heads, timeout);

  boost::posix_time::milliseconds n1(100);
  //boost::this_thread::sleep(n1);

  md->send(NewConcurrentMessageDispatcher::EVAL_IN_MQ, dlv_eval->getInQueue(), heads, timeout);
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



void
send_input_belief_state(NewConcurrentMessageDispatcherPtr md, 
			std::size_t neighbor_offset,
			ReturnedBeliefStateList& rbs)
{
  int timeout = 0;

  ForwardMessage* notification = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_OUT_MQ, neighbor_offset, timeout);
  
  std::size_t qid = notification->query_id;

  std::size_t ctx_id = ctxid_from_qid(qid);
  std::size_t query_order = qorder_from_qid(qid);

  std::size_t no_rbs = rbs.size();
  NewJoinIn* ji = new NewJoinIn(neighbor_offset, no_rbs);

  md->send<NewJoinIn>(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, ji, timeout);
  
  for (ReturnedBeliefStateList::const_iterator it = rbs.begin(); it != rbs.end(); ++it)
    {
      md->send<ReturnedBeliefState>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_offset, *it, timeout);
    }
}


BOOST_AUTO_TEST_CASE ( testRunningIntermediateContext )
{
  std::size_t NO_BS = 4;
  std::size_t BS_SIZE = 3;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BS, BS_SIZE);

  std::size_t QUEUE_SIZE = 5;
  std::size_t NO_NEIGHBORS = 2;
  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(QUEUE_SIZE, NO_NEIGHBORS));
  ConcurrentMessageQueuePtr cmq0(new ConcurrentMessageQueue(QUEUE_SIZE));
  ConcurrentMessageQueuePtr cmq1(new ConcurrentMessageQueue(QUEUE_SIZE));
  md->registerMQ(cmq0, NewConcurrentMessageDispatcher::JOIN_IN_MQ, 0);
  md->registerMQ(cmq1, NewConcurrentMessageDispatcher::JOIN_IN_MQ, 1);

  // needs some time for md to start up all of its MQ, otherwise we get some segfaults
  boost::posix_time::milliseconds md_starting_up(100);
  boost::this_thread::sleep(md_starting_up);

  std::size_t ctx_id0 = 0;
  Belief epsilon0(ctx_id0, "epsilon");
  Belief a(ctx_id0, "a");
  Belief a1(ctx_id0, "a1");
  Belief a2(ctx_id0, "a2");
  BeliefTablePtr btab0(new BeliefTable);

  ID id_epsilon0 = btab0->storeAndGetID(epsilon0);
  ID id_a = btab0->storeAndGetID(a);
  ID id_a1 = btab0->storeAndGetID(a1);
  ID id_a2 = btab0->storeAndGetID(a2);

  std::size_t ctx_id1 = 1;
  Belief epsilon1(ctx_id1, "epsilon");
  Belief b(ctx_id1, "b");
  Belief b1(ctx_id1, "b1");
  Belief b2(ctx_id1, "b2");
  BeliefTablePtr btab1(new BeliefTable);

  ID id_epsilon1 = btab1->storeAndGetID(epsilon1);
  ID id_b = btab1->storeAndGetID(b);
  ID id_b1 = btab1->storeAndGetID(b1);
  ID id_b2 = btab1->storeAndGetID(b2);

  std::size_t ctx_id2 = 2;
  Belief epsilon2(ctx_id2, "epsilon");
  Belief c(ctx_id2, "c");
  Belief d(ctx_id2, "d");
  Belief e(ctx_id2, "e");
  BeliefTablePtr btab2(new BeliefTable);

  ID id_epsilon2 = btab2->storeAndGetID(epsilon2);
  ID id_c = btab2->storeAndGetID(c);
  ID id_d = btab2->storeAndGetID(d);
  ID id_e = btab2->storeAndGetID(e);

  std::size_t ctx_id3 = 3;
  Belief epsilon3(ctx_id3, "epsilon");
  Belief f(ctx_id3, "f");
  Belief g(ctx_id3, "g");
  Belief h(ctx_id3, "h");
  BeliefTablePtr btab3(new BeliefTable);

  ID id_epsilon3 = btab3->storeAndGetID(epsilon3);
  ID id_f = btab3->storeAndGetID(f);
  ID id_g = btab3->storeAndGetID(g);
  ID id_h = btab3->storeAndGetID(h);

  // Initialize input belief states
  // C2:
  // \epsilon, {b}, \epsilon, {\neg f, g}
  NewBeliefState* bs21 = new NewBeliefState(NO_BS, BS_SIZE);
  bs21->setEpsilon(ctx_id1, bso->getStartingOffsets());
  bs21->set(ctx_id1, id_b.address, bso->getStartingOffsets());
  bs21->setEpsilon(ctx_id3, bso->getStartingOffsets());
  bs21->set(ctx_id3, id_f.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs21->set(ctx_id3, id_g.address, bso->getStartingOffsets());

  // \epsilon, {b}, \epsilon, {f, \neg g}
  NewBeliefState* bs22 = new NewBeliefState(NO_BS, BS_SIZE);
  bs22->setEpsilon(ctx_id1, bso->getStartingOffsets());
  bs22->set(ctx_id1, id_b.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs22->setEpsilon(ctx_id3, bso->getStartingOffsets());
  bs22->set(ctx_id3, id_f.address, bso->getStartingOffsets());
  bs22->set(ctx_id3, id_g.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);

  std::size_t qid = query_id(ctx_id0, 0);
  ReturnedBeliefState* rbs21 = new ReturnedBeliefState(bs21, qid);
  ReturnedBeliefState* rbs22 = new ReturnedBeliefState(bs22, qid);
  ReturnedBeliefState* rbs23 = new ReturnedBeliefState(NULL, qid);
  ReturnedBeliefStateList rbs2;
  rbs2.push_back(rbs21);
  rbs2.push_back(rbs22);
  rbs2.push_back(rbs23);

  // C3:
  // \epsilon, \epsilon, {c}, {\neg f, g}
  NewBeliefState* bs31 = new NewBeliefState(NO_BS, BS_SIZE);
  bs31->setEpsilon(ctx_id2, bso->getStartingOffsets());
  bs31->set(ctx_id2, id_c.address, bso->getStartingOffsets());
  bs31->setEpsilon(ctx_id3, bso->getStartingOffsets());
  bs31->set(ctx_id3, id_f.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs31->set(ctx_id3, id_g.address, bso->getStartingOffsets());

  // \epsilon, \epsilon, {\neg c}, {\neg f, g}
  NewBeliefState* bs32 = new NewBeliefState(NO_BS, BS_SIZE);
  bs32->setEpsilon(ctx_id2, bso->getStartingOffsets());
  bs32->set(ctx_id2, id_c.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs32->setEpsilon(ctx_id3, bso->getStartingOffsets());
  bs32->set(ctx_id3, id_f.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs32->set(ctx_id3, id_g.address, bso->getStartingOffsets());

  // \epsilon, \epsilon, {\neg c}, {f, \neg g}
  NewBeliefState* bs33 = new NewBeliefState(NO_BS, BS_SIZE);
  bs33->setEpsilon(ctx_id2, bso->getStartingOffsets());
  bs33->set(ctx_id2, id_c.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs33->setEpsilon(ctx_id3, bso->getStartingOffsets());
  bs33->set(ctx_id3, id_f.address, bso->getStartingOffsets());
  bs33->set(ctx_id3, id_g.address, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);

  ReturnedBeliefState* rbs31 = new ReturnedBeliefState(bs31, qid);
  ReturnedBeliefState* rbs32 = new ReturnedBeliefState(bs32, qid);
  ReturnedBeliefState* rbs33 = new ReturnedBeliefState(bs33, qid);
  ReturnedBeliefState* rbs34 = new ReturnedBeliefState(NULL, qid);
  ReturnedBeliefStateList rbs3;
  rbs3.push_back(rbs31);
  rbs3.push_back(rbs32);
  rbs3.push_back(rbs33);
  rbs3.push_back(rbs34);

  std::size_t noff0 = 0;
  std::size_t noff1 = 1;

  std::size_t nindex0 = 0;
  std::size_t nindex1 = 1;

  boost::thread send_from_2_thread(send_input_belief_state, md, noff0, rbs2);
  boost::thread send_from_3_thread(send_input_belief_state, md, noff1, rbs3);

  ReturnedBeliefState* rbs41 = new ReturnedBeliefState(NULL, qid);
  ReturnedBeliefStateList rbs4;
  rbs4.push_back(rbs41);

  ReturnedBeliefState* rbs51 = new ReturnedBeliefState(NULL, qid);
  ReturnedBeliefStateList rbs5;
  rbs5.push_back(rbs51);  

  Tuple bridge_body;
  bridge_body.push_back(id_b);
  bridge_body.push_back(id_c);
  BridgeRule br(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_a, bridge_body);

  BridgeRuleTablePtr brtab(new BridgeRuleTable);
  ID id_br = brtab->storeAndGetID(br);

  NewNeighborPtr neighbor1(new NewNeighbor(nindex0, noff0, "localhost", "5001"));
  NewNeighborPtr neighbor2(new NewNeighbor(nindex1, noff1, "localhost", "5002"));
  NewNeighborVecPtr neighbors(new NewNeighborVec);
  neighbors->push_back(neighbor1);
  neighbors->push_back(neighbor2);

  NeighborOffset2IndexPtr o2i(new NeighborOffset2Index);
  o2i->insert(std::pair<std::size_t, std::size_t>(noff0, nindex0));
  o2i->insert(std::pair<std::size_t, std::size_t>(noff1, nindex1));

  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //std::string kbspec(ex);
  //kbspec += "/testRunningContext.inp";

  std::string kbspec = "../../examples/testRunningContext.inp";

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);

  std::size_t pack_size = 10;
  NewContext ctx(ctx_id0, pack_size, dlv_inst, btab0, brtab, neighbors, o2i);

  NewJoinerDispatcherPtr joiner_dispatcher(new NewJoinerDispatcher(md));
  joiner_dispatcher->registerIdOffset(qid, ctx_id0);
  
  boost::thread joiner_dispatcher_thread(*joiner_dispatcher);
  boost::thread context_thread(ctx, md, joiner_dispatcher);

  // needs some time for the context thread to start up and register its REQUEST_MQ to md
  boost::posix_time::milliseconds context_starting_up(100);
  boost::this_thread::sleep(context_starting_up);  

  std::size_t parent_qid = query_id(100, 1);
  ForwardMessage* fwd_mess = new ForwardMessage(parent_qid, 1, 5);
  ForwardMessage* end_mess = new ForwardMessage(shutdown_query_id(), 1, 5);

  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id0, fwd_mess, timeout);
  md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id0, end_mess, timeout);

  send_from_2_thread.join();
  send_from_3_thread.join();

  boost::thread send_ending_from_2_thread(send_input_belief_state, md, noff0, rbs4);
  boost::thread send_ending_from_3_thread(send_input_belief_state, md, noff1, rbs5);

  send_ending_from_2_thread.join();
  send_ending_from_3_thread.join();
  context_thread.join();

  ReturnedBeliefState* res1 = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, timeout);
  ReturnedBeliefState* res2 = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, timeout);

  std::cerr << "res1 = " << *res1 << std::endl;
  std::cerr << "res2 = " << *res2 << std::endl;
}



BOOST_AUTO_TEST_CASE ( testRunningLeafContext )
{
  std::size_t NO_BS = 2;
  std::size_t BS_SIZE = 10;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BS, BS_SIZE);

  std::string kbspec;
  BeliefTablePtr btab(new BeliefTable);

  std::size_t QUEUE_SIZE = 10;
  std::size_t NO_NEIGHBORS = 0;

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(QUEUE_SIZE, NO_NEIGHBORS));

  std::size_t ctx_id1 = 1;
  init_local_kb(ctx_id1, kbspec, btab);

  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);

  NewContext ctx(ctx_id1, dlv_inst, btab);
  NewJoinerDispatcherPtr joiner_dispatcher = NewJoinerDispatcherPtr();

  boost::thread context_thread(ctx, md, joiner_dispatcher);

  // needs some time for the context thread to start up and register its REQUEST_MQ to md
  boost::posix_time::milliseconds context_starting_up(100);
  boost::this_thread::sleep(context_starting_up);

  std::size_t k11 = 3;
  std::size_t k12 = 5;
  std::size_t k21 = 0;
  std::size_t k22 = 0;
  std::size_t k31 = 1;
  std::size_t k32 = 10;

  std::size_t parent_qid = query_id(0, 1);
  ForwardMessage* request1 = new ForwardMessage(parent_qid, k11, k12);
  ForwardMessage* request2 = new ForwardMessage(parent_qid, k21, k22);
  ForwardMessage* request3 = new ForwardMessage(parent_qid, k31, k32);
  ForwardMessage* end_request = new ForwardMessage(shutdown_query_id(), 1, 5);
  int timeout = 0;

  md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id1, request1, timeout);
  md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id1, request2, timeout);
  md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id1, request3, timeout);
  md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, ctx_id1, end_request, timeout);

  std::size_t count1 = read_output_dispatcher(md);
  std::size_t count2 = read_output_dispatcher(md);
  std::size_t count3 = read_output_dispatcher(md);

  BOOST_CHECK_EQUAL(count1, k12 - k11 + 1);
  BOOST_CHECK_EQUAL(count2, 6);
  BOOST_CHECK_EQUAL(count3, 6);

  context_thread.join();
}


// Local Variables:
// mode: C++
// End:
