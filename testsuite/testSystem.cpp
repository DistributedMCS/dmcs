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
#include "network/NewClient.h"
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
run_client(std::string server_port, ForwardMessage& want_send)
{
  std::string header = HEADER_REQ_DMCS;
  std::string host_name = "localhost";
  boost::asio::io_service io_service_client;
  boost::asio::ip::tcp::resolver resolver(io_service_client);
  boost::asio::ip::tcp::resolver::query query(host_name, server_port);
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint_client = *it;

  NewClient c(io_service_client, it, header, want_send);
  io_service_client.run();
  io_service_client.reset();

  std::size_t invoker = invoker_from_qid(want_send.qid);
  std::size_t neighbor_id = neighbor_id_from_qid(want_send.qid);
  std::size_t end_qid = shutdown_query_id(invoker, neighbor_id);
  ForwardMessage end_mess(end_qid);

  c.terminate(end_mess);
  io_service_client.run();

  std::cerr << "exit from run_client" << std::endl;
}

BOOST_AUTO_TEST_CASE ( testLeafSystem )
{
  std::size_t SYSTEM_SIZE = 2;
  std::size_t BS_SIZE = 10;
  std::size_t QUEUE_SIZE = 10;

  std::string kbspec;
  BeliefTablePtr btab(new BeliefTable);

  // setup a leaf context
  std::size_t invoker0 = 1000;
  std::size_t ctx_id1 = 0;
  init_local_kb(ctx_id1, kbspec, btab);

  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);
  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);

  std::cerr << "Starting context..." << std::endl;
  NewContextPtr ctx(new NewContext(ctx_id1, dlv_inst, btab));
  NewContextVecPtr ctxs(new NewContextVec);
  ctxs->push_back(ctx);
  
  RegistryPtr reg(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, ctxs));

  std::cerr << "Starting server..." << std::endl;
  // start the server
  std::string port = "5555";
  std::size_t server_port = 5555;
  boost::thread server_thread(run_server, server_port, reg);

  boost::posix_time::milliseconds server_starting_up(200);
  boost::this_thread::sleep(server_starting_up);

  std::size_t query_order1 = 1;
  std::size_t qid1 = query_id(invoker0, ctx_id1, query_order1);

  std::size_t k1 = 1;
  std::size_t k2 = 5;

  ForwardMessage want_send(qid1, k1, k2);

  std::cerr << "Starting client..." << std::endl;
  boost::thread client_thread(run_client, port, want_send);

  client_thread.join();
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
  std::size_t qid1 = query_id(invoker0, ctx_id1, query_order1);

  std::size_t k1 = 1;
  std::size_t k2 = 5;

  ForwardMessage want_send(qid1, k1, k2);

  std::cerr << "Starting client..." << std::endl;
  boost::thread client_thread(run_client, port1, want_send);

  client_thread.join();
}


BOOST_AUTO_TEST_CASE ( testDiamondPlusSystem )
{
  std::size_t SYSTEM_SIZE = 6;
  std::size_t BS_SIZE = 10;
  std::size_t QUEUE_SIZE = 10;

  std::size_t ctx_id1 = 1;
  std::size_t ctx_id2 = 2;
  std::size_t ctx_id3 = 3;
  std::size_t ctx_id4 = 4;
  std::size_t ctx_id5 = 5;

  std::string ctx_hostname1 = "localhost";
  std::string ctx_hostname2 = "localhost";
  std::string ctx_hostname3 = "localhost";
  std::string ctx_hostname4 = "localhost";
  std::string ctx_hostname5 = "localhost";

  std::string port1 = "7001";
  std::string port2 = "7002";
  std::string port3 = "7003";
  std::string port4 = "7004";
  std::string port5 = "7005";

  std::size_t ctx_port1 = 7001;
  std::size_t ctx_port2 = 7002;
  std::size_t ctx_port3 = 7003;
  std::size_t ctx_port4 = 7004;
  std::size_t ctx_port5 = 7005;

  std::size_t pack_size = 5;

  /************************** CONTEXT 5 **************************/
  BeliefTablePtr btab55(new BeliefTable);

  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //kbspec5 = ex;
  //kbspec5 += "/context5.lp";
  std::string kbspec5 = "../../examples/context5.lp";

  Belief belief_epsilon55(ctx_id5, "epsilon");
  Belief belief_pc1c4_55(ctx_id5, "p(c1,c4)");
  Belief belief_pc2c5_55(ctx_id5, "p(c2,c5)");
  Belief belief_pc3c6_55(ctx_id5, "p(c3,c6)");
  Belief belief_ec1c4_55(ctx_id5, "e(c1,c4)");
  Belief belief_ec2c5_55(ctx_id5, "e(c2,c5)");
  Belief belief_ec3c6_55(ctx_id5, "e(c3,c6)");
  Belief belief_fc1c4_55(ctx_id5, "f(c1,c4)");
  Belief belief_fc2c5_55(ctx_id5, "f(c2,c5)");
  Belief belief_fc3c6_55(ctx_id5, "f(c3,c6)");


  ID id_epsilon55 = btab55->storeAndGetID(belief_epsilon55);
  ID id_pc1c4_55  = btab55->storeAndGetID(belief_pc1c4_55);
  ID id_pc2c5_55  = btab55->storeAndGetID(belief_pc2c5_55);
  ID id_pc3c6_55  = btab55->storeAndGetID(belief_pc3c6_55);
  ID id_ec1c4_55  = btab55->storeAndGetID(belief_ec1c4_55);
  ID id_ec2c5_55  = btab55->storeAndGetID(belief_ec2c5_55);
  ID id_ec3c6_55  = btab55->storeAndGetID(belief_ec3c6_55);
  ID id_fc1c4_55  = btab55->storeAndGetID(belief_fc1c4_55);
  ID id_fc2c5_55  = btab55->storeAndGetID(belief_fc2c5_55);
  ID id_fc3c6_55  = btab55->storeAndGetID(belief_fc3c6_55);

  EnginePtr dlv_engine5 = DLVEngine::create();
  EngineWPtr dlv_engine_wp5(dlv_engine5);
  InstantiatorPtr dlv_inst5 = dlv_engine5->createInstantiator(dlv_engine_wp5, kbspec5);

  NewContextPtr ctx5(new NewContext(ctx_id5, dlv_inst5, btab55));
  NewContextVecPtr contexts5(new NewContextVec);
  contexts5->push_back(ctx5);

  RegistryPtr reg5(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts5));
  

  boost::thread server_thread5(run_server, ctx_port5, reg5);
  boost::posix_time::milliseconds server_starting_up5(200);
  boost::this_thread::sleep(server_starting_up5);

  /************************** CONTEXT 4 **************************/
  BeliefTablePtr btab44(new BeliefTable);
  //kbspec4 = ex;
  //kbspec4 += "/context4.lp";
  std::string kbspec4 = "../../examples/context4.lp";

  Belief belief_epsilon44(ctx_id4, "epsilon");
  Belief belief_dc1c4_44(ctx_id4, "d(c1,c4)");
  Belief belief_dc2c5_44(ctx_id4, "d(c2,c5)");
  Belief belief_dc3c6_44(ctx_id4, "d(c3,c6)");
  Belief belief_dprimec1c4_44(ctx_id4, "dprime(c1,c4)");
  Belief belief_dprimec2c5_44(ctx_id4, "dprime(c2,c5)");
  Belief belief_dprimec3c6_44(ctx_id4, "dprime(c3,c6)");

  ID id_epsilon44 = btab44->storeAndGetID(belief_epsilon44);
  ID id_dc1c4 = btab44->storeAndGetID(belief_dc1c4_44);
  ID id_dc2c5 = btab44->storeAndGetID(belief_dc2c5_44);
  ID id_dc3c6 = btab44->storeAndGetID(belief_dc3c6_44);
  ID id_dprimec1c4 = btab44->storeAndGetID(belief_dprimec1c4_44);
  ID id_dprimec2c5 = btab44->storeAndGetID(belief_dprimec2c5_44);
  ID id_dprimec3c6 = btab44->storeAndGetID(belief_dprimec3c6_44);

  BeliefTablePtr btab54(new BeliefTable);
  Belief belief_epsilon54(ctx_id5, "epsilon");
  Belief belief_ec1c4_54(ctx_id5, "e(c1,c4)");
  Belief belief_ec2c5_54(ctx_id5, "e(c2,c5)");
  Belief belief_ec3c6_54(ctx_id5, "e(c3,c6)");
  Belief belief_fc1c4_54(ctx_id5, "f(c1,c4)");
  Belief belief_fc2c5_54(ctx_id5, "f(c2,c5)");
  Belief belief_fc3c6_54(ctx_id5, "f(c3,c6)");

  ID id_epsilon54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 0);
  ID id_ec1c4_54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 4);
  ID id_ec2c5_54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 5);
  ID id_ec3c6_54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 6);
  ID id_fc1c4_54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 7);
  ID id_fc2c5_54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 8);
  ID id_fc3c6_54 = ID(ID::MAINKIND_BELIEF | ctx_id5, 9);

  BOOST_CHECK_EQUAL(id_epsilon54, id_epsilon55);
  BOOST_CHECK_EQUAL(id_ec1c4_54, id_ec1c4_55);
  BOOST_CHECK_EQUAL(id_ec2c5_54, id_ec2c5_55);
  BOOST_CHECK_EQUAL(id_ec3c6_54, id_ec3c6_55);
  BOOST_CHECK_EQUAL(id_fc1c4_54, id_fc1c4_55);
  BOOST_CHECK_EQUAL(id_fc2c5_54, id_fc2c5_55);
  BOOST_CHECK_EQUAL(id_fc3c6_54, id_fc3c6_55);
  
  // add beliefs to btab54 using the new method
  btab54->storeWithID(belief_epsilon54, id_epsilon54);
  btab54->storeWithID(belief_ec1c4_54, id_ec1c4_54);
  btab54->storeWithID(belief_ec2c5_54, id_ec2c5_54);
  btab54->storeWithID(belief_ec3c6_54, id_ec3c6_54);
  btab54->storeWithID(belief_fc1c4_54, id_fc1c4_54);
  btab54->storeWithID(belief_fc2c5_54, id_fc2c5_54);
  btab54->storeWithID(belief_fc3c6_54, id_fc3c6_54);

  // bridge rules C5 --> C4
  BridgeRuleTablePtr bridge_rules4(new BridgeRuleTable);

  // adding all ground instances of d(X,Y) :- (5:f(X,Y)).
  //
  // d(c1,c4) :- (5:f(c1,c4)).
  Tuple body14;
  body14.push_back(id_fc1c4_54);
  BridgeRule br14(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_dc1c4_54, body14);
  bridge_rules4->storeAndGetID(br14);

  // d(c2,c5) :- (5:f(c2,c5)).
  Tuple body24;
  body24.push_back(id_fc2c5_54);
  BridgeRule br24(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_dc2c5_54, body24);
  bridge_rules4->storeAndGetID(br24);
  
  // d(c3,c6) :- (5:f(c3,c6)).
  Tuple body34;
  body34.push_back(id_fc3c6_54);
  BridgeRule br34(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_dc3c6_54, body34);
  bridge_rules4->storeAndGetID(br34);

  std::size_t ctx_off54 = 0;
  NewNeighborPtr neighbor54(new NewNeighbor(ctx_id5, ctx_off54, ctx_hostname5, port5));
  NewNeighborVecPtr neighbors4(new NewNeighborVec);
  neighbors4->push_back(neighbor54);

  EnginePtr dlv_engine4 = DLVEngine::create();
  EngineWPtr dlv_engine_wp4(dlv_engine4);
  InstantiatorPtr dlv_inst4 = dlv_engine4->createInstantiator(dlv_engine_wp4, kbspec4);

  NewContextPtr ctx4(new NewContext(ctx_id4, pack_size, dlv_inst4, btab4, bridge_rules4, neighbors4));
  NewContextVecPtr contexts4(new NewContextVec);
  contexts4->push_back(ctx4);

  RegistryPtr reg4(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts5));
  boost::thread server_thread4(run_server, ctx_port4, reg4);

  /************************** CONTEXT 3 **************************/  
  BeliefTablePtr btab33(new BeliefTable);
  //kbspec3 = ex;
  //kbspec3 += "/context3.lp";
  std::string kbspec3 = "../../examples/context3.lp";

  Belief belief_epsilon33(ctx_id3, "epsilon");
  Belief belief_cc1c4_33(ctx_id3, "c(c1,c4)");
  Belief belief_cc2c5_33(ctx_id3, "c(c2,c5)");
  Belief belief_cc3c6_33(ctx_id3, "c(c3,c6)");

  ID id_epsilon33 = btab33->storeAndGetID(belief_epsilon33);
  ID id_cc1c4_33 = btab33->storeAndGetID(belief_cc1c4_33);
  ID id_cc2c5_33 = btab33->storeAndGetID(belief_cc3c5_33);
  ID id_cc3c6_33 = btab33->storeAndGetID(belief_cc4c6_33);

  BeliefTablePtr btab53(new BeliefTable);
  Belief belief_epsilon53(ctx_id5, "epsilon");
  Belief belief_ec1c4_53(ctx_id5, "e(c1,c4)");
  Belief belief_ec2c5_53(ctx_id5, "e(c2,c5)");
  Belief belief_ec3c6_53(ctx_id5, "e(c3,c6)");
  Belief belief_fc1c4_53(ctx_id5, "f(c1,c4)");
  Belief belief_fc2c5_53(ctx_id5, "f(c2,c5)");
  Belief belief_fc3c6_53(ctx_id5, "f(c3,c6)");

  ID id_epsilon53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 0);
  ID id_ec1c4_53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 4);
  ID id_ec2c5_53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 5);
  ID id_ec3c6_53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 6);
  ID id_fc1c4_53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 7);
  ID id_fc2c5_53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 8);
  ID id_fc3c6_53 = ID(ID::MAINKIND_BELIEF | ctx_id5, 9);

  BOOST_CHECK_EQUAL(id_epsilon53, id_epsilon55);
  BOOST_CHECK_EQUAL(id_ec1c4_53, id_ec1c4_55);
  BOOST_CHECK_EQUAL(id_ec2c5_53, id_ec2c5_55);
  BOOST_CHECK_EQUAL(id_ec3c6_53, id_ec3c6_55);
  BOOST_CHECK_EQUAL(id_fc1c4_53, id_fc1c4_55);
  BOOST_CHECK_EQUAL(id_fc2c5_53, id_fc2c5_55);
  BOOST_CHECK_EQUAL(id_fc3c6_53, id_fc3c6_55);

  // add beliefs to btab53 using the new method
  btab53->storeWithID(belief_epsilon53, id_epsilon53);
  btab53->storeWithID(belief_ec1c4_53, id_ec1c4_53);
  btab53->storeWithID(belief_ec2c5_53, id_ec2c5_53);
  btab53->storeWithID(belief_ec3c6_53, id_ec3c6_53);
  btab53->storeWithID(belief_fc1c4_53, id_fc1c4_53);
  btab53->storeWithID(belief_fc2c5_53, id_fc2c5_53);
  btab53->storeWithID(belief_fc3c6_53, id_fc3c6_53);

  // bridge rules C5 --> C3
  BridgeRuleTablePtr bridge_rules3(new BridgeRuleTable);

  // adding all ground instances of c(C,Y) :- (5:e(X,Y));
  //
  // c(c1,c4) :- (5:e(c1,c4)).
  Tuple body13;
  body13.push_back(id_ec1c4_53);
  BridgeRule br13(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_cc1c4_33, body13);
  bridge_rules3->storeAndGetID(br13);

  // c(c2,c5) :- (5:e(c2,c5)).
  Tuple body23;
  body23.push_back(id_ec2c5_53);
  BridgeRule br23(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_cc2c5_33, body23);
  bridge_rules3->storeAndGetID(br23);

  // c(c3,c6) :- (5:e(c3,c6)).
  Tuple body33;
  body33.push_back(id_ec3c6_53);
  BridgeRule br33(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_cc3c6_33, body33);
  bridge_rules3->storeAndGetID(br33);

  std::size_t ctx_off53 = 0;
  NewNeighborPtr neighbor53(new NewNeighbor(ctx_id5, ctx_off53, ctx_hostname5, port5));
  NewNeighborVecPtr neighbors3(new NewNeighborVec);
  neighbors3->push_back(neighbor53);

  EnginePtr dlv_engine3 = DLVEngine::create();
  EngineWPtr dlv_engine_wp3(dlv_engine3);
  InstantiatorPtr dlv_inst3 = dlv_engine3->createInstantiator(dlv_engine_wp3, kbspec3);

  NewContextPtr ctx3(new NewContext(ctx_id3, pack_size, dlv_inst3, btab3, bridge_rules3, neighbors3));
  NewContextVecPtr contexts3(new NewContextVec);
  contexts3->push_back(ctx3);

  RegistryPtr reg3(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts3));
  boost::thread server_thread3(run_server, ctx_port3, reg3);

  /************************** CONTEXT 2 **************************/
  BeliefTablePtr btab22(new BeliefTable);
  //kbspec2 = ex;
  //kbspec2 += "/context2.lp";
  std::string kbspec2 = "../../examples/context2.lp";

  Belief belief_epsilon22(ctx_id2, "epsilon");
  Belief belief_bc4_22(ctx_id2, "b(c4)");
  Belief belief_bc5_22(ctx_id2, "b(c5)");
  Belief belief_bc6_22(ctx_id2, "b(c6)");
  Belief belief_domc1c4_22(ctx_id2, "dom(c1,c4)");
  Belief belief_domc2c5_22(ctx_id2, "dom(c2,c5)");
  Belief belief_domc3c6_22(ctx_id2, "dom(c3,c6)");

  ID id_epsilon22 = btab22->storeAndGetID(belief_epsilon22);
  ID id_bc4_22 = btab22->storeAndGetID(belief_bc4_22);
  ID id_bc5_22 = btab22->storeAndGetID(belief_bc5_22);
  ID id_bc6_22 = btab22->storeAndGetID(belief_bc6_22);
  ID id_domc1c4_22 = btab22->storeAndGetID(belief_domc1c4_22);
  ID id_domc2c5_22 = btab22->storeAndGetID(belief_domc2c5_22);
  ID id_domc3c6_22 = btab22->storeAndGetID(belief_domc3c6_22);

  BeliefTablePtr btab32(new BeliefTable);
  ID id_epsilon32 = ID(ID::MAINKIND_BELIEF | ctx_id3, 0);
  ID id_cc1c4_32 = ID(ID::MAINKIND_BELIEF | ctx_id3, 1);
  ID id_cc2c5_32 = ID(ID::MAINKIND_BELIEF | ctx_id3, 2);
  ID id_cc3c6_32 = ID(ID::MAINKIND_BELIEF | ctx_id3, 3);

  BOOST_CHECK_EQUAL(id_epsilon32, id_epsilon33);
  BOOST_CHECK_EQUAL(id_cc1c4_32, id_cc1c4_33);
  BOOST_CHECK_EQUAL(id_cc2c5_32, id_cc2c5_33);
  BOOST_CHECK_EQUAL(id_cc3c6_32, id_cc3c6_33);

  // add beliefs to btab32 using the new method
  btab32->storeWithID(belief_epsilon32, id_epsilon32);
  btab32->storeWithID(belief_cc1c4_32, id_cc1c4_32);
  btab32->storeWithID(belief_cc2c5_32, id_cc2c5_32);
  btab32->storeWithID(belief_cc3c6_32, id_cc3c6_32);

  // bridge rules C3 --> C2
  BridgeRuleTablePtr bridge_rules2(new BridgeRuleTable);
  
  // adding all ground instances of b(Y) :- (2:dom(X,Y)), not (3:c(X,Y)).
  //
  // b(c4) :- (2:dom(c1,c4)), not (3:c(c1,c4)).
  Tuple body12;
  body12.push_back(id_domc1c4_22);
  body12.push_back(nafLiteralFromBelief(id_cc1c4_32));
  BridgeRule br12(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_bc4_22, body12);
  bridge_rules2->storeAndGetID(br12);

  // b(c5) :- (2:dom(c2,c5)), not (3:c(c2,c5)).
  Tuple body22;
  body22.push_back(id_domc2c5_22);
  body22.push_back(nafLiteralFromBelief(id_cc2c5_32));
  BridgeRule br22(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_bc5_22, body22);
  bridge_rules2->storeAndGetID(br22);

  // b(c6) :- (2:dom(c3,c6)), not (3:c(c3,c6)).
  Tuple body32;
  body32.push_back(id_domc3c6_22);
  body32.push_back(nafLiteralFromBelief(id_cc3c6_32));
  BridgeRule br32(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_bc6_22, body32);
  bridge_rules2->storeAndGetID(br32);

  std::size_t ctx_off32 = 0;
  NewNeighborPtr neighbor32(new NewNeighbor(ctx_id3, ctx_off32, ctx_hostname3, port3));
  NewNeighborVecPtr neighbors2(new NewNeighborVec);
  neighbors2->push_back(neighbor32);

  EnginePtr dlv_engine2 = DLVEngine::create();
  EngineWPtr dlv_engine_wp2(dlv_engine2);
  InstantiatorPtr dlv_inst2 = dlv_engine2->createInstantiator(dlv_engine_wp2, kbspec2);

  NewContextPtr ctx2(new NewContext(ctx_id2, pack_size, dlv_inst2, btab22, bridge_rules2, neighbors2));
  NewContextVecPtr contexts2(new NewContextVec);
  contexts2->push_back(ctx2);

  RegistryPtr reg2(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts2));
  boost::thread server_thread2(run_server, ctx_port2, reg2);

  /************************** CONTEXT 1 **************************/  
  BeliefTablePtr btab11(new BeliefTable);
  //kbspec2 = ex;
  //kbspec2 += "/context2.lp";
  std::string kbspec2 = "../../examples/context2.lp";

  Belief belief_epsilon11(ctx_id1, "epsilon");
  Belief belief_ac1_11(ctx_id1, "a(c1)");
  Belief belief_ac2_11(ctx_id1, "a(c2)");
  Belief belief_ac3_11(ctx_id1, "a(c3)");

  ID id_epsilon11 = btab11->storeAndGetID(belief_epsilon11);
  ID id_ac1_11 = btab11->storeAndGetID(belief_ac1_11);
  ID id_ac2_11 = btab11->storeAndGetID(belief_ac2_11);
  ID id_ac3_11 = btab11->storeAndGetID(belief_ac3_11);

  BeliefTablePtr btab31(new BeliefTable);
  ID id_epsilon31 = ID(ID::MAINKIND_BELIEF | ctx_id3, 0);
  ID id_cc1c4_31 = ID(ID::MAINKIND_BELIEF | ctx_id3, 1);
  ID id_cc2c5_31 = ID(ID::MAINKIND_BELIEF | ctx_id3, 2);
  ID id_cc3c6_31 = ID(ID::MAINKIND_BELIEF | ctx_id3, 3);

  BOOST_CHECK_EQUAL(id_epsilon31, id_epsilon33);
  BOOST_CHECK_EQUAL(id_cc1c4_31, id_cc1c4_33);
  BOOST_CHECK_EQUAL(id_cc2c5_31, id_cc2c5_33);
  BOOST_CHECK_EQUAL(id_cc3c6_31, id_cc3c6_33);

  // add beliefs to btab31 using the new method
  btab31->storeWithID(belief_epsilon31, id_epsilon31);
  btab31->storeWithID(belief_cc1c4_31, id_cc1c4_31);
  btab31->storeWithID(belief_cc2c5_31, id_cc2c5_31);
  btab31->storeWithID(belief_cc3c6_31, id_cc3c6_31);

  BeliefTablePtr btab41(new BeliefTable);
  ID id_epsilon41 = ID(ID::MAINKIND_BELIEF | ctx_id4, 0);
  ID id_dprimec1c4_41 = ID(ID::MAINKIND_BELIEF | ctx_id4, 4);
  ID id_dprimec2c5_41 = ID(ID::MAINKIND_BELIEF | ctx_id4, 5);
  ID id_dprimec3c6_41 = ID(ID::MAINKIND_BELIEF | ctx_id4, 6);

  BOOST_CHECK_EQUAL(id_epsilon41, id_epsilon44);
  BOOST_CHECK_EQUAL(id_dprimec1c4_41, id_dprimec1c4_44);
  BOOST_CHECK_EQUAL(id_dprimec2c5_41, id_dprimec2c5_44);
  BOOST_CHECK_EQUAL(id_dprimec3c6_41, id_dprimec3c6_44);

  // add beliefs to btab31 using the new method
  btab41->storeWithID(belief_epsilon41, id_epsilon41);
  btab41->storeWithID(belief_dprimec1c4_41, id_dprimec1c4_41);
  btab41->storeWithID(belief_dprimec2c5_41, id_dprimec2c5_41);
  btab41->storeWithID(belief_dprimec3c6_41, id_dprimec3c6_41);

  // bridge rules C3,C4 --> C1
  BridgeRuleTablePtr bridge_rules1(new BridgeRuleTable);

  // adding all ground instances of a(X) :- (3:c(X,Y)), not (4:dprime(X,Y)).
  //
  // a(c1) :- (3:c(c1,c4)), not (4:dprime(c1,c4)).
  Tuple body11;
  body11.push_back(id_cc1c4_31);
  body11.push_back(nafLiteralFromBelief(id_dprimec1c4_41));
  BridgeRule br11(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_ac1_11, body11);
  bridge_rules1->storeAndGetID(br11);

  // a(c2) :- (3:c(c2,c5)), not (4:dprime(c2,c5)).
  Tuple body21;
  body21.push_back(id_cc2c5_31);
  body21.push_back(nafLiteralFromBelief(id_dprimec2c5_41));
  BridgeRule br21(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_ac2_11, body21);
  bridge_rules1->storeAndGetID(br21); 

  // a(c3) :- (3:c(c3,c6)), not (4:dprime(c3,c6)).
  Tuple body31;
  body31.push_back(id_cc3c6_31);
  body31.push_back(nafLiteralFromBelief(id_dprimec3c6_41));
  BridgeRule br31(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_ac3_11, body31);
  bridge_rules1->storeAndGetID(br31);

  std::size_t ctx_off31 = 0;
  std::size_t ctx_off41 = 1;
  NewNeighborPtr neighbor31(new NewNeighbor(ctx_id3, ctx_off31, ctx_hostname3, port3));
  NewNeighborPtr neighbor41(new NewNeighbor(ctx_id4, ctx_off41, ctx_hostname4, port4));
  NewNeighborVecPtr neighbors1(new NewNeighborVec);
  neighbors1->push_back(neighbor31);
  neighbors1->push_back(neighbor41);

  EnginePtr dlv_engine1 = DLVEngine::create();
  EngineWPtr dlv_engine_wp1(dlv_engine1);
  InstantiatorPtr dlv_inst1 = dlv_engine1->createInstantiator(dlv_engine_wp1, kbspec1);

  NewContextPtr ctx1(new NewContext(ctx_id1, pack_size, dlv_inst1, btab11, bridge_rules1, neighbors1));
  NewContextVecPtr contexts1(new NewContextVec);
  contexts1->push_back(ctx1);

  RegistryPtr reg1(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts1));
  boost::thread server_thread1(run_server, ctx_port1, reg1);

  boost::posix_time::milliseconds servers_starting_up(500);
  boost::this_thread::sleep(servers_starting_up);

  std::size_t invoker0 = 1000;
  std::size_t query_order1 = 1;
  std::size_t qid1 = query_id(invoker0, ctx_id1, query_order1);

  std::size_t k1 = 1;
  std::size_t k2 = 5;

  ForwardMessage want_send(qid1, k1, k2);

  std::cerr << "Starting client..." << std::endl;
  boost::thread client_thread(run_client, port1, want_send);

  client_thread.join();
}

// Local Variables:
// mode: C++
// End:
