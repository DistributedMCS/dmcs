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
 * @file   testSystem.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Mar  13 9:28:15 2012
 * 
 * @brief  
 * 
 * 
 */

#include "DmcsClientTest.h"

#include "mcs/BeliefStateOffset.h"
#include "mcs/BeliefTable.h"
#include "mcs/NewContext.h"
#include "dmcs/DLVEngine.h"
#include "dmcs/DLVInstantiator.h"
#include "dmcs/DLVEvaluator.h"
#include "mcs/RequestDispatcher.h"
#include "network/NewConcurrentMessageDispatcher.h"

#include "network/NewServer.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSystem"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

using namespace dmcs;

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

  std::cerr << "Initializing belief table." << std::endl;
  btab->storeAndGetID(belief_epsilon1);
  btab->storeAndGetID(belief_a);
  btab->storeAndGetID(belief_b);
  btab->storeAndGetID(belief_c);
  btab->storeAndGetID(belief_d);
  btab->storeAndGetID(belief_e);
  btab->storeAndGetID(belief_f);
  btab->storeAndGetID(belief_g);
  btab->storeAndGetID(belief_h);
  btab->storeAndGetID(belief_i);
  btab->storeAndGetID(belief_j);
  std::cerr << "Initializing belief table: DONE." << std::endl;
}


void
run_server(std::size_t server_port, const RegistryPtr reg)
{
  boost::asio::io_service io_service_server;
  boost::asio::ip::tcp::endpoint endpoint_server(boost::asio::ip::tcp::v4(), server_port);

  NewServer s(io_service_server, endpoint_server, reg);
  boost::this_thread::interruption_point();
  io_service_server.run();
  
  std::cerr << "exit from run_server" << std::endl;
}


void
run_client(std::string server_port, ForwardMessage* ws1, ForwardMessage* ws2)
{
  std::string host_name = "localhost";
  boost::asio::io_service io_service_client;
  boost::asio::ip::tcp::resolver resolver(io_service_client);
  boost::asio::ip::tcp::resolver::query query(host_name, server_port);
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint_client = *it;

  DmcsClientTest c(io_service_client, it, ws1, ws2);
  io_service_client.run();

  std::cerr << "exit from run_client" << std::endl;
}

BOOST_AUTO_TEST_CASE ( testLeafSystem )
{
#if 0
  std::size_t NO_BS = 2;
  std::size_t BS_SIZE = 10;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BS, BS_SIZE);

  std::string kbspec;
  BeliefTablePtr btab(new BeliefTable);

  std::size_t QUEUE_SIZE = 10;
  std::size_t NO_NEIGHBORS = 0;

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(QUEUE_SIZE, NO_NEIGHBORS));

  // setup a leaf context
  std::size_t invoker0 = 0;
  std::size_t ctx_id1 = 1;
  init_local_kb(ctx_id1, kbspec, btab);

  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);


  NewOutputDispatcherPtr output_dispatcher(new NewOutputDispatcher(md));
  std::cerr << "Starting output dispatcher thread... " << output_dispatcher.get() << std::endl;
  NewOutputDispatcherWrapper output_dispatcher_wrapper;
  boost::thread* output_dispatcher_thread = new boost::thread(output_dispatcher_wrapper, output_dispatcher);


  std::cerr << "Starting request dispatcher thread..." << std::endl;
  RequestDispatcherPtr request_dispatcher(new RequestDispatcher(md));
  RequestDispatcherWrapper request_dispatcher_wrapper;
  boost::thread* request_dispatcher_thread = new boost::thread(request_dispatcher_wrapper, request_dispatcher);

  std::cerr << "Starting context..." << std::endl;
  NewContextPtr ctx(new NewContext(ctx_id1, dlv_inst, btab));
  NewContextWrapper context_wrapper;
  boost::thread* context_thread = new boost::thread(context_wrapper, ctx, md, request_dispatcher);

  // needs some time for the context thread to start up and register its REQUEST_MQ to md
  boost::posix_time::milliseconds context_starting_up(100);
  boost::this_thread::sleep(context_starting_up);
  RegistryPtr reg(new Registry(md, output_dispatcher));

  std::cerr << "Starting server..." << std::endl;
  // start the server
  std::string port = "5555";
  std::size_t server_port = 5555;
  boost::thread* server_thread = new boost::thread(run_server, server_port, reg);

  boost::posix_time::milliseconds server_starting_up(200);
  boost::this_thread::sleep(server_starting_up);

  std::size_t query_order1 = 1;
  std::size_t query_order2 = 2;
  std::size_t qid1 = query_id(invoker0, ctx_id1, query_order1);
  std::size_t qid2 = query_id(invoker0, ctx_id1, query_order2);

  std::size_t k11 = 1;
  std::size_t k12 = 5;
  std::size_t k21 = 2;
  std::size_t k22 = 7;

  ForwardMessage* ws1 = new ForwardMessage(qid1, k11, k12);
  ForwardMessage* ws2 = new ForwardMessage(qid1, k21, k22);

  std::cerr << "Starting client..." << std::endl;
  boost::thread* client_thread = new boost::thread(run_client, port, ws1, ws2);


  client_thread->join();
  std::cerr << "Join client." << std::endl;

  context_thread->join();
  std::cerr << "Join context." << std::endl;

  request_dispatcher_thread->interrupt();
  request_dispatcher_thread->join();
  std::cerr << "Join request dispatcher." << std::endl;

  output_dispatcher_thread->interrupt();
  output_dispatcher_thread->join();
  std::cerr << "Join output dispatcher." << std::endl;

  server_thread->interrupt();
  server_thread->join();
  std::cerr << "Join server." << std::endl;
#endif
}


void
init_leaf_ctx(std::size_t leaf_id,
	      std::string& leaf_kbspec,
	      BeliefTablePtr leaf_btab)
{
  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //leaf_kbspec = ex;
  //leaf_kbspec += "/leafContext.inp";
  leaf_kbspec = "../../examples/leafContext.inp";


  Belief belief_epsilon2(leaf_id, "epsilon");
  Belief belief_d(leaf_id, "d");
  Belief belief_e(leaf_id, "e");

  leaf_btab->storeAndGetID(belief_epsilon2);
  leaf_btab->storeAndGetID(belief_d);
  leaf_btab->storeAndGetID(belief_e);
}



void
init_root_ctx(std::size_t root_id,
	      std::string& root_kbspec,
	      BeliefTablePtr root_btab,
	      BeliefTablePtr leaf_btab,
	      BridgeRuleTablePtr bridge_rules)
{
  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //root_kbspec = ex;
  //root_kbspec += "/rootContext.inp";
  root_kbspec = "../../examples/rootContext.inp";


  Belief belief_epsilon1(root_id, "epsilon");
  Belief belief_a(root_id, "a");
  Belief belief_b(root_id, "b");
  Belief belief_c(root_id, "c");

  root_btab->storeAndGetID(belief_epsilon1);
  root_btab->storeAndGetID(belief_a);
  root_btab->storeAndGetID(belief_b);
  root_btab->storeAndGetID(belief_c);

  // bridge rules
  std::string str_c = "c";
  std::string str_d = "d";
  ID id_c = root_btab->getIDByString(str_c);
  ID id_d = leaf_btab->getIDByString(str_d);

  Tuple body1;
  body1.push_back(id_d);

  // c :- (2:d)
  BridgeRule br1(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_c, body1);
  bridge_rules->storeAndGetID(br1);  
}





BOOST_AUTO_TEST_CASE ( testIntermediateSystem )
{
  std::size_t SYSTEM_SIZE = 2;
  std::size_t BS_SIZE = 10;
  std::size_t QUEUE_SIZE = 10;

  /************************** LEAF CONTEXT **************************/
  std::string kbspec2;
  BeliefTablePtr btab2(new BeliefTable);

  std::size_t ctx_id2 = 1;
  std::string ctx_hostname2 = "localhost";
  std::string port2 = "5678";
  std::size_t ctx_port2 = 5678;

  init_leaf_ctx(ctx_id2, kbspec2, btab2);

  EnginePtr dlv_engine2 = DLVEngine::create();
  EngineWPtr dlv_engine_wp2(dlv_engine2);
  InstantiatorPtr dlv_inst2 = dlv_engine2->createInstantiator(dlv_engine_wp2, kbspec2);

  NewContextPtr ctx2(new NewContext(ctx_id2, dlv_inst2, btab2));
  NewContextVecPtr contexts2(new NewContextVec);
  contexts2->push_back(ctx2);

  RegistryPtr reg2(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts2));

  boost::thread server_thread2(run_server, ctx_port2, reg2);
  boost::posix_time::milliseconds server_starting_up2(200);
  boost::this_thread::sleep(server_starting_up2);

  /************************** ROOT CONTEXT **************************/
  std::string kbspec1;
  BeliefTablePtr btab1(new BeliefTable);
  BridgeRuleTablePtr br1(new BridgeRuleTable);

  std::size_t invoker = 1000;
  std::size_t ctx_id1 = 0;
  init_root_ctx(ctx_id1, kbspec1, btab1, btab2, br1);

  std::size_t ctx_off2 = 0;
  
  NewNeighborPtr n1(new NewNeighbor(ctx_id2, ctx_off2, ctx_hostname2, port2));
  NewNeighborVecPtr neighbors1(new NewNeighborVec);
  neighbors1->push_back(n1);

  EnginePtr dlv_engine1 = DLVEngine::create();
  EngineWPtr dlv_engine_wp1(dlv_engine1);
  InstantiatorPtr dlv_inst1 = dlv_engine1->createInstantiator(dlv_engine_wp1, kbspec1);

  std::size_t pack_size = 3;
  NewContextPtr ctx1(new NewContext(ctx_id1, pack_size, dlv_inst1, btab1, br1, neighbors1));
  NewContextVecPtr contexts1(new NewContextVec);
  contexts1->push_back(ctx1);

  RegistryPtr reg1(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts1, neighbors1));
  std::string port1 = "5120";
  std::size_t ctx_port1 = 5120;
  boost::thread server_thread1(run_server, ctx_port1, reg1);

  boost::posix_time::milliseconds server_starting_up1(200);
  boost::this_thread::sleep(server_starting_up1);

  std::size_t invoker0 = 1000;
  std::size_t query_order1 = 1;
  std::size_t query_order2 = 2;
  std::size_t qid1 = query_id(invoker0, ctx_id1, query_order1);
  std::size_t qid2 = query_id(invoker0, ctx_id1, query_order2);

  std::size_t k11 = 1;
  std::size_t k12 = 5;
  std::size_t k21 = 2;
  std::size_t k22 = 7;

  ForwardMessage* ws1 = new ForwardMessage(qid1, k11, k12);
  ForwardMessage* ws2 = new ForwardMessage(qid1, k21, k22);

  std::cerr << "Starting client..." << std::endl;
  boost::thread client_thread(run_client, port1, ws1, ws2);  

  client_thread.join();
}


// Local Variables:
// mode: C++
// End:
