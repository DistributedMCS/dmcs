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
 * @file   testSystemWQueryPlan.cpp
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
#include "mcs/ID.h"
#include "mcs/NewContext.h"
#include "dmcs/DLVEngine.h"
#include "dmcs/DLVInstantiator.h"
#include "dmcs/DLVEvaluator.h"
#include "mcs/RequestDispatcher.h"
#include "network/NewClient.h"
#include "network/NewConcurrentMessageDispatcher.h"
#include "network/NewServer.h"
#include "mcs/QueryPlan.h"
#include "parser/QueryPlanParser.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testSystemWithQueryPlan"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#define INSTANTIATE 1

using namespace dmcs;

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

#if 0
BOOST_AUTO_TEST_CASE ( testLeafSystemWithQueryPlan )
{
  std::size_t SYSTEM_SIZE = 2;
  std::size_t BS_SIZE = 10;
  std::size_t QUEUE_SIZE = 10;

  std::string kbspec = "../../examples/abcdContext.inp";
  BeliefTablePtr btab(new BeliefTable);

  // setup a leaf context
  std::size_t invoker0 = 1000;
  std::size_t ctx_id1 = 0;
  std::string ctx_hostname1 = "localhost";
  std::string port1 = "5001";
  std::size_t ctx_port1 = 5001;

  const char* queryplan =
    "["
    "  {"
    "    ContextId: 0,"
    "    LocalSignature:"
    "    {"
    "      1:  [a],"
    "      2:  [b],"
    "      3:  [c],"
    "      4:  [d],"
    "      5:  [e],"
    "      6:  [f],"
    "      7:  [g],"
    "      8:  [h],"
    "      9:  [i],"
    "      10: [j],"
    "    },"
    "  },"
    "]";

  ContextQueryPlanMapPtr qp0 = QueryPlanParser::parseString(queryplan);
  const ContextQueryPlan& at0ctx0 = qp0->find(0)->second;

  //  init_local_kb(ctx_id1, kbspec, btab);


  EnginePtr dlv_engine = DLVEngine::create();
  EngineWPtr dlv_engine_wp(dlv_engine);

  InstantiatorPtr dlv_inst = dlv_engine->createInstantiator(dlv_engine_wp, kbspec);


  std::cerr << "Starting context..." << std::endl;
  NewContextPtr ctx(new NewContext(ctx_id1, dlv_inst, at0ctx0.localSignature));
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
#endif


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


#if 0
BOOST_AUTO_TEST_CASE ( testIntermediateSystem )
{
  std::size_t SYSTEM_SIZE = 2;
  std::size_t BS_SIZE = 10;
  std::size_t QUEUE_SIZE = 10;

  std::string kbspec1 = "../../examples/rootContext.inp";
  std::string kbspec2 = "../../examples/leafContext.inp";

  std::size_t invoker = 1000;
  std::size_t ctx_id1 = 0;
  std::size_t ctx_id2 = 1;
  std::string ctx_hostname2 = "localhost";
  std::string port2 = "5678";
  std::size_t ctx_port2 = 5678;

  const char* queryplan_leaf =
    "["
    "  {"
    "    ContextId: 1,"
    "    LocalSignature:"
    "    {"
    "      1: [d],"
    "    },"
    "  },"
    "]";

  const char* queryplan_root =
    "["
    "  {"
    "    ContextId: 0,"
    "    LocalSignature:"
    "    {"
    "      1:  [a],"
    "      2:  [b],"
    "      3:  [c],"
    "    },"
    "  },"
    "  {"
    "    ContextId: 1,"
    "    InputSignature:"
    "    {"
    "      1: [d],"
    "      2: [e],"
    "    },"
    "  },"
    "]";

  /************************** LEAF CONTEXT **************************/
  ContextQueryPlanMapPtr qp_leaf = QueryPlanParser::parseString(queryplan_leaf);
  const ContextQueryPlan& at1ctx1 = qp_leaf->find(1)->second;

  EnginePtr dlv_engine2 = DLVEngine::create();
  EngineWPtr dlv_engine_wp2(dlv_engine2);
  InstantiatorPtr dlv_inst2 = dlv_engine2->createInstantiator(dlv_engine_wp2, kbspec2);

  NewContextPtr ctx2(new NewContext(ctx_id2, dlv_inst2, at1ctx1.localSignature));
  NewContextVecPtr contexts2(new NewContextVec);
  contexts2->push_back(ctx2);

  RegistryPtr reg2(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts2));

  boost::thread server_thread2(run_server, ctx_port2, reg2);
  boost::posix_time::milliseconds server_starting_up2(200);
  boost::this_thread::sleep(server_starting_up2);

  /************************** ROOT CONTEXT **************************/
  ContextQueryPlanMapPtr qp_root = QueryPlanParser::parseString(queryplan_root);
  const ContextQueryPlan& at0ctx0 = qp_root->find(0)->second;
  const ContextQueryPlan& at0ctx1 = qp_root->find(1)->second;

  BridgeRuleTablePtr br1(new BridgeRuleTable);
  std::string str_c = "c";
  std::string str_d = "d";
  ID id_c = at0ctx0.localSignature->getIDByString(str_c);
  ID id_d = at0ctx1.groundInputSignature->getIDByString(str_d);  

  Tuple body1;
  body1.push_back(id_d);

  // c :- (2:d)
  BridgeRule r1(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_c, body1);
  br1->storeAndGetID(r1);

  std::size_t ctx_off2 = 0;
  NewNeighborPtr n1(new NewNeighbor(ctx_id2, ctx_off2, ctx_hostname2, port2));
  NewNeighborVecPtr neighbors1(new NewNeighborVec);
  neighbors1->push_back(n1);

  EnginePtr dlv_engine1 = DLVEngine::create();
  EngineWPtr dlv_engine_wp1(dlv_engine1);
  InstantiatorPtr dlv_inst1 = dlv_engine1->createInstantiator(dlv_engine_wp1, kbspec1);

  std::size_t pack_size = 3;
  NewContextPtr ctx1(new NewContext(ctx_id1, pack_size, dlv_inst1, at0ctx0.localSignature, br1, neighbors1));
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
#endif




// check if belief strings in localSignature of one query plan has same id as
// belief string in inputSignature of other query plan
//
// ctxIdx must be the same, only first query plan is local query plan,
// other query plan only knows part of the context
//inline void CHECK_BeliefIDs(
//    ContextQueryPlanMapConstPtr inputQP, ContextQueryPlanMapConstPtr localQP, 
//    unsigned ctxIdx, const char* belief)
#define CHECK_BeliefIDs(inputQP,localQP,ctxIdx,belief) \
{ \
  ContextQueryPlanMap::const_iterator iti = inputQP->find(ctxIdx); \
  BOOST_CHECK(iti != inputQP->end()); \
  const ContextQueryPlan& inputPlan = iti->second; \
  BOOST_CHECK(!!inputPlan.groundInputSignature); \
  \
  ContextQueryPlanMap::const_iterator itl = localQP->find(ctxIdx); \
  BOOST_CHECK(itl != localQP->end()); \
  const ContextQueryPlan& localPlan = itl->second; \
  BOOST_CHECK(!!localPlan.localSignature); \
  \
  BOOST_CHECK_EQUAL(localPlan.localSignature->getIDByString(belief), \
                    inputPlan.groundInputSignature->getIDByString(belief)); \
}


BOOST_AUTO_TEST_CASE ( testDiamondPlusSystem )
{
  std::size_t SYSTEM_SIZE = 6;
  std::size_t BS_SIZE = 10;
  std::size_t QUEUE_SIZE = 10;

  std::size_t invoker = 1023;
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

  // query plan for context 1
  const char* queryPlan1 =
  "["
  "  {"
  "    ContextId: 1,"
  "    HostName: \"localhost\","
  "    Port: 5001," 
  "    LocalSignature:" 
  "    {" 
  "      1: [a, c1]," 
  "      2: [a, c2]," 
  "      3: [a, c3]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 3,"
  "    HostName: \"localhost\","
  "    Port: 5003," 
  "    InputSignature:" 
  "    {" 
  "      1: [c, c1, c4]," 
  "      2: [c, c2, c5]," 
  "      3: [c, c3, c6]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 4,"
  "    HostName: \"localhost\","
  "    Port: 5004," 
  "    InputSignature:" 
  "    {" 
  "      4: [dprime, c1, c4]," 
  "      5: [dprime, c2, c5]," 
  "      6: [dprime, c3, c6]," 
  "    }," 
  "  }," 
  "]";
  const char* brfile1 =
  "a(c1) :- (3:c(c1,c4)), not (4:dprime(c1,c4)).\n";
  "a(c2) :- (3:c(c2,c5)), not (4:dprime(c2,c5)).\n";
  "a(c3) :- (3:c(c3,c6)), not (4:dprime(c3,c6)).\n";

  // query plan for context 2
  const char* queryPlan2 =
  "["
  "  {"
  "    ContextId: 2,"
  "    HostName: \"localhost\","
  "    Port: 5002," 
  "    LocalSignature:" 
  "    {" 
  "      1: [b, c4]," 
  "      2: [b, c5]," 
  "      3: [b, c6]," 
  "      4: [dom, c1, c4]," 
  "      5: [dom, c2, c5]," 
  "      6: [dom, c3, c6]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 3,"
  "    HostName: \"localhost\","
  "    Port: 5003," 
  "    InputSignature:" 
  "    {" 
  "      1: [c, c1, c4]," 
  "      2: [c, c2, c5]," 
  "      3: [c, c3, c6]," 
  "    }," 
  "  }," 
  "]";
  const char* brfile2 =
  "b(c4) :- (2:dom(c1,c4)), not (3:c(c1,c4)).\n";
  "b(c5) :- (2:dom(c2,c5)), not (3:c(c2,c5)).\n";
  "b(c6) :- (2:dom(c3,c6)), not (3:c(c3,c6)).\n";

  // query plan for context 3
  const char* queryPlan3 =
  "["
  "  {"
  "    ContextId: 3,"
  "    HostName: \"localhost\","
  "    Port: 5003," 
  "    LocalSignature:" 
  "    {" 
  "      1: [c, c1, c4]," 
  "      2: [c, c2, c5]," 
  "      3: [c, c3, c6]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 5,"
  "    HostName: \"localhost\","
  "    Port: 5005," 
  "    InputSignature:" 
  "    {" 
  "      4: [e, c1, c4]," 
  "      5: [e, c2, c5]," 
  "      6: [e, c3, c6]," 
  "      7: [f, c1, c4]," 
  "      8: [f, c2, c5]," 
  "      9: [f, c3, c6]," 
  "    }," 
  "  }," 
  "]";
  const char* brfile3 =
  "c(c1,c4) :- (5:e(c1,c4)).\n";
  "c(c2,c5) :- (5:e(c2,c5)).\n";
  "c(c3,c6) :- (5:e(c3,c6)).\n";

  // query plan for context 4
  const char* queryPlan4 =
  "["
  "  {"
  "    ContextId: 4,"
  "    HostName: \"localhost\","
  "    Port: 5004," 
  "    LocalSignature:" 
  "    {" 
  "      1: [d, c1, c4]," 
  "      2: [d, c2, c5]," 
  "      3: [d, c3, c6]," 
  "      4: [dprime, c1, c4]," 
  "      5: [dprime, c2, c5]," 
  "      6: [dprime, c3, c6]," 
  "    }," 
  "  }," 
  "  {"
  "    ContextId: 5,"
  "    HostName: \"localhost\","
  "    Port: 5005," 
  "    InputSignature:" 
  "    {" 
  "      4: [e, c1, c4]," 
  "      5: [e, c2, c5]," 
  "      6: [e, c3, c6]," 
  "      7: [f, c1, c4]," 
  "      8: [f, c2, c5]," 
  "      9: [f, c3, c6]," 
  "    }," 
  "  }," 
  "]";
  const char* brfile4 =
  "d(c1,c4) :- (5:f(c1,c4)).\n";
  "d(c2,c5) :- (5:f(c2,c5)).\n";
  "d(c3,c6) :- (5:f(c3,c6)).\n";

  // query plan for context 5
  const char* queryPlan5 =
  "["
  "  {"
  "    ContextId: 5,"
  "    HostName: \"localhost\","
  "    Port: 5005,"
  "    LocalSignature:" 
  "    {" 
  "      1: [p, c1, c4]," 
  "      2: [p, c2, c5]," 
  "      3: [p, c3, c6]," 
  "      4: [e, c1, c4]," 
  "      5: [e, c2, c5]," 
  "      6: [e, c3, c6]," 
  "      7: [f, c1, c4]," 
  "      8: [f, c2, c5]," 
  "      9: [f, c3, c6]," 
  "    }," 
  "  }," 
  "]";
  const char* brfile5 = "";

  // context 1
  std::string str_ac1 = "a(c1)";
  std::string str_ac2 = "a(c2)";
  std::string str_ac3 = "a(c3)";

  // context 2
  std::string str_bc4 = "b(c4)";
  std::string str_bc5 = "b(c5)";
  std::string str_bc6 = "b(c6)";
  std::string str_domc1c4 = "dom(c1,c4)";
  std::string str_domc2c5 = "dom(c2,c5)";
  std::string str_domc3c6 = "dom(c3,c6)";

  // context 3
  std::string str_cc1c4 = "c(c1,c4)";
  std::string str_cc2c5 = "c(c2,c5)";
  std::string str_cc3c6 = "c(c3,c6)";

  // context 4
  std::string str_dc1c4 = "d(c1,c4)";
  std::string str_dc2c5 = "d(c2,c5)";
  std::string str_dc3c6 = "d(c3,c6)";
  std::string str_dprimec1c4 = "dprime(c1,c4)";
  std::string str_dprimec2c5 = "dprime(c2,c5)";
  std::string str_dprimec3c6 = "dprime(c3,c6)";

  // context 5
  std::string str_pc1c4 = "p(c1,c4)";
  std::string str_pc2c5 = "p(c2,c5)";
  std::string str_pc3c6 = "p(c3,c6)";
  std::string str_ec1c4 = "e(c1,c4)";
  std::string str_ec2c5 = "e(c2,c5)";
  std::string str_ec3c6 = "e(c3,c6)";
  std::string str_fc1c4 = "f(c1,c4)";
  std::string str_fc2c5 = "f(c2,c5)";
  std::string str_fc3c6 = "f(c3,c6)";

  /************************** CONTEXT 5 **************************/
  ContextQueryPlanMapPtr qp5 = QueryPlanParser::parseString(queryPlan5);
  const ContextQueryPlan& at5ctx5 = qp5->find(5)->second;

  //const char* ex = getenv("EXAMPLESDIR");
  //assert (ex != 0);
  //kbspec5 = ex;
  //kbspec5 += "/context5.lp";
  std::string kbspec5 = "../../examples/context4.lp";

  EnginePtr dlv_engine5 = DLVEngine::create();
  EngineWPtr dlv_engine_wp5(dlv_engine5);
  InstantiatorPtr dlv_inst5 = dlv_engine5->createInstantiator(dlv_engine_wp5, kbspec5);

  NewBeliefState* interface54(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  NewBeliefState* interface53(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  ReturnPlanMapPtr return_plan5(new ReturnPlanMap);
  return_plan5->insert(std::make_pair<std::size_t, NewBeliefState*>(ctx_id4, interface54));
  return_plan5->insert(std::make_pair<std::size_t, NewBeliefState*>(ctx_id3, interface53));

  NewContextPtr ctx5(new NewContext(ctx_id5, dlv_inst5, at5ctx5.localSignature, return_plan5));
  NewContextVecPtr contexts5(new NewContextVec);
  contexts5->push_back(ctx5);

  RegistryPtr reg5(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts5));
  
  boost::thread server_thread5(run_server, ctx_port5, reg5);
  boost::posix_time::milliseconds server_starting_up5(200);
  boost::this_thread::sleep(server_starting_up5);


  /************************** CONTEXT 4 **************************/
  ContextQueryPlanMapPtr qp4 = QueryPlanParser::parseString(queryPlan4);
  const ContextQueryPlan& at4ctx4 = qp4->find(4)->second;
  const ContextQueryPlan& at4ctx5 = qp4->find(5)->second;

  //kbspec4 = ex;
  //kbspec4 += "/context4.lp";
  std::string kbspec4 = "../../examples/context3.lp";

  CHECK_BeliefIDs(qp4, qp5, 5, "e(c1,c4)");
  CHECK_BeliefIDs(qp4, qp5, 5, "e(c2,c5)");
  CHECK_BeliefIDs(qp4, qp5, 5, "e(c3,c6)");
  CHECK_BeliefIDs(qp4, qp5, 5, "f(c1,c4)");
  CHECK_BeliefIDs(qp4, qp5, 5, "f(c2,c5)");
  CHECK_BeliefIDs(qp4, qp5, 5, "f(c3,c6)");

  #if 0
  // query plan of ctx 4 must be sufficient to parse bridge rules!
  BridgeRuleTablePtr bridge_rules4 = BridgeRuleParser::parseString(brfile4, qp4);
  #else
  // bridge rules C5 --> C4
  BridgeRuleTablePtr bridge_rules4(new BridgeRuleTable);

  // adding all ground instances of d(X,Y) :- (5:f(X,Y)).
  //
  // d(c1,c4) :- (5:f(c1,c4)).
  ID id_dc1c4_44 = at4ctx4.localSignature->getIDByString(str_dc1c4);
  ID id_dc2c5_44 = at4ctx4.localSignature->getIDByString(str_dc2c5);
  ID id_dc3c6_44 = at4ctx4.localSignature->getIDByString(str_dc3c6);
  ID id_fc1c4_54 = at4ctx5.groundInputSignature->getIDByString(str_fc1c4);
  ID id_fc2c5_54 = at4ctx5.groundInputSignature->getIDByString(str_fc2c5);
  ID id_fc3c6_54 = at4ctx5.groundInputSignature->getIDByString(str_fc3c6);
  Tuple body14;
  body14.push_back(id_fc1c4_54);
  BridgeRule br14(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_dc1c4_44, body14);
  bridge_rules4->storeAndGetID(br14);

  // d(c2,c5) :- (5:f(c2,c5)).
  Tuple body24;
  body24.push_back(id_fc2c5_54);
  BridgeRule br24(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_dc2c5_44, body24);
  bridge_rules4->storeAndGetID(br24);
  
  // d(c3,c6) :- (5:f(c3,c6)).
  Tuple body34;
  body34.push_back(id_fc3c6_54);
  BridgeRule br34(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_dc3c6_44, body34);
  bridge_rules4->storeAndGetID(br34);
  #endif

  //  #if INSTANTIATE
  std::size_t ctx_off54 = 0;
  NewNeighborPtr neighbor54(new NewNeighbor(ctx_id5, ctx_off54, ctx_hostname5, port5));
  NewNeighborVecPtr neighbors4(new NewNeighborVec);
  neighbors4->push_back(neighbor54);
  NewNeighborVecPtr neighbors_reg4(new NewNeighborVec);
  neighbors_reg4->push_back(neighbor54);

  EnginePtr dlv_engine4 = DLVEngine::create();
  EngineWPtr dlv_engine_wp4(dlv_engine4);
  InstantiatorPtr dlv_inst4 = dlv_engine4->createInstantiator(dlv_engine_wp4, kbspec4);

  NewBeliefState* interface41(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  ReturnPlanMapPtr return_plan4(new ReturnPlanMap);
  return_plan4->insert(std::make_pair<std::size_t, NewBeliefState*>(ctx_id1, interface41));

  NewContextPtr ctx4(new NewContext(ctx_id4, pack_size, dlv_inst4, at4ctx4.localSignature, return_plan4, bridge_rules4, neighbors4));
  NewContextVecPtr contexts4(new NewContextVec);
  contexts4->push_back(ctx4);

  RegistryPtr reg4(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts4, neighbors_reg4));
  boost::thread server_thread4(run_server, ctx_port4, reg4);
  //  #endif


  /************************** CONTEXT 3 **************************/  
  ContextQueryPlanMapPtr qp3 = QueryPlanParser::parseString(queryPlan3);
  const ContextQueryPlan& at3ctx3 = qp3->find(3)->second;
  const ContextQueryPlan& at3ctx5 = qp3->find(5)->second;

  //kbspec3 = ex;
  //kbspec3 += "/context3.lp";
  std::string kbspec3 = "../../examples/context2.lp";

  #if 0
  // query plan of ctx 3 must be sufficient to parse bridge rules!
  BridgeRuleTablePtr bridge_rules3 = BridgeRuleParser::parseString(brfile3, qp3);
  #else
  // bridge rules C5 --> C3
  BridgeRuleTablePtr bridge_rules3(new BridgeRuleTable);

  ID id_cc1c4_33 = at3ctx3.localSignature->getIDByString(str_cc1c4);
  ID id_cc2c5_33 = at3ctx3.localSignature->getIDByString(str_cc2c5);
  ID id_cc3c6_33 = at3ctx3.localSignature->getIDByString(str_cc3c6);
  ID id_ec1c4_53 = at3ctx5.groundInputSignature->getIDByString(str_ec1c4);
  ID id_ec2c5_53 = at3ctx5.groundInputSignature->getIDByString(str_ec2c5);
  ID id_ec3c6_53 = at3ctx5.groundInputSignature->getIDByString(str_ec3c6);
  // adding all ground instances of c(X,Y) :- (5:e(X,Y));
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
  #endif

  //  #if INSTANTIATE
  std::size_t ctx_off53 = 0;
  NewNeighborPtr neighbor53(new NewNeighbor(ctx_id5, ctx_off53, ctx_hostname5, port5));
  NewNeighborVecPtr neighbors3(new NewNeighborVec);
  neighbors3->push_back(neighbor53);
  NewNeighborVecPtr neighbors_reg3(new NewNeighborVec);
  neighbors_reg3->push_back(neighbor53);

  EnginePtr dlv_engine3 = DLVEngine::create();
  EngineWPtr dlv_engine_wp3(dlv_engine3);
  InstantiatorPtr dlv_inst3 = dlv_engine3->createInstantiator(dlv_engine_wp3, kbspec3);

  NewBeliefState* interface31(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  NewBeliefState* interface32(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  ReturnPlanMapPtr return_plan3(new ReturnPlanMap);
  return_plan3->insert(std::make_pair<std::size_t, NewBeliefState*>(ctx_id1, interface31));
  return_plan3->insert(std::make_pair<std::size_t, NewBeliefState*>(ctx_id2, interface32));

  NewContextPtr ctx3(new NewContext(ctx_id3, pack_size, dlv_inst3, at3ctx3.localSignature, return_plan3, bridge_rules3, neighbors3));
  NewContextVecPtr contexts3(new NewContextVec);
  contexts3->push_back(ctx3);

  RegistryPtr reg3(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts3, neighbors_reg3));
  boost::thread server_thread3(run_server, ctx_port3, reg3);
  //  #endif

  /************************** CONTEXT 2 **************************/
  ContextQueryPlanMapPtr qp2 = QueryPlanParser::parseString(queryPlan2);
  const ContextQueryPlan& at2ctx2 = qp2->find(2)->second;
  const ContextQueryPlan& at2ctx3 = qp2->find(3)->second;

  //kbspec2 = ex;
  //kbspec2 += "/context2.lp";
  std::string kbspec2 = "../../examples/context1.lp";

  #if 0
  // query plan of ctx 2 must be sufficient to parse bridge rules!
  BridgeRuleTablePtr bridge_rules2 = BridgeRuleParser::parseString(brfile2, qp2);
  #else
  // bridge rules C3 --> C2
  BridgeRuleTablePtr bridge_rules2(new BridgeRuleTable);

  ID id_bc4_22 = at2ctx2.localSignature->getIDByString(str_bc4);
  ID id_bc5_22 = at2ctx2.localSignature->getIDByString(str_bc5);
  ID id_bc6_22 = at2ctx2.localSignature->getIDByString(str_bc6);
  ID id_domc1c4_22 = at2ctx2.localSignature->getIDByString(str_domc1c4);
  ID id_domc2c5_22 = at2ctx2.localSignature->getIDByString(str_domc2c5);
  ID id_domc3c6_22 = at2ctx2.localSignature->getIDByString(str_domc3c6);
  ID id_cc1c4_32 = at2ctx3.groundInputSignature->getIDByString(str_cc1c4);
  ID id_cc2c5_32 = at2ctx3.groundInputSignature->getIDByString(str_cc2c5);
  ID id_cc3c6_32 = at2ctx3.groundInputSignature->getIDByString(str_cc3c6);
  // adding all ground instances of b(Y) :- (2:dom(X,Y)), not (3:c(X,Y)).
  //
  // b(c4) :- (2:dom(c1,c4)), not (3:c(c1,c4)).
  Tuple body12;
  body12.push_back(id_domc1c4_22);
  body12.push_back(ID::nafLiteralFromBelief(id_cc1c4_32));
  BridgeRule br12(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_bc4_22, body12);
  bridge_rules2->storeAndGetID(br12);

  // b(c5) :- (2:dom(c2,c5)), not (3:c(c2,c5)).
  Tuple body22;
  body22.push_back(id_domc2c5_22);
  body22.push_back(ID::nafLiteralFromBelief(id_cc2c5_32));
  BridgeRule br22(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_bc5_22, body22);
  bridge_rules2->storeAndGetID(br22);

  // b(c6) :- (2:dom(c3,c6)), not (3:c(c3,c6)).
  Tuple body32;
  body32.push_back(id_domc3c6_22);
  body32.push_back(ID::nafLiteralFromBelief(id_cc3c6_32));
  BridgeRule br32(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_bc6_22, body32);
  bridge_rules2->storeAndGetID(br32);
  #endif

  //  #if INSTANTIATE
  std::size_t ctx_off32 = 0;
  NewNeighborPtr neighbor32(new NewNeighbor(ctx_id3, ctx_off32, ctx_hostname3, port3));
  NewNeighborVecPtr neighbors2(new NewNeighborVec);
  neighbors2->push_back(neighbor32);
  NewNeighborVecPtr neighbors_reg2(new NewNeighborVec);
  neighbors_reg2->push_back(neighbor32);

  EnginePtr dlv_engine2 = DLVEngine::create();
  EngineWPtr dlv_engine_wp2(dlv_engine2);
  InstantiatorPtr dlv_inst2 = dlv_engine2->createInstantiator(dlv_engine_wp2, kbspec2);

  NewBeliefState* interface20(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  ReturnPlanMapPtr return_plan2(new ReturnPlanMap);
  return_plan2->insert(std::make_pair<std::size_t, NewBeliefState*>(invoker, interface20));

  NewContextPtr ctx2(new NewContext(ctx_id2, pack_size, dlv_inst2, at2ctx2.localSignature, return_plan2, bridge_rules2, neighbors2));
  NewContextVecPtr contexts2(new NewContextVec);
  contexts2->push_back(ctx2);

  RegistryPtr reg2(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts2, neighbors_reg2));
  boost::thread server_thread2(run_server, ctx_port2, reg2);
  //  #endif

  /************************** CONTEXT 1 **************************/  
  ContextQueryPlanMapPtr qp1 = QueryPlanParser::parseString(queryPlan1);
  const ContextQueryPlan& at1ctx1 = qp1->find(1)->second;
  const ContextQueryPlan& at1ctx3 = qp1->find(3)->second;
  const ContextQueryPlan& at1ctx4 = qp1->find(4)->second;

  //kbspec1 = ex;
  //kbspec1 += "/context1.lp";
  std::string kbspec1 = "../../examples/context0.lp";

  // instead of this complicated lines
  BOOST_CHECK_EQUAL(at1ctx3.groundInputSignature->getIDByString("c(c1,c4)"),
                    at3ctx3.localSignature->getIDByString("c(c1,c4)"));

  // we do this (here and in the following)
  CHECK_BeliefIDs(qp1, qp3, 3, "c(c1,c4)");
  CHECK_BeliefIDs(qp1, qp3, 3, "c(c2,c5)");
  CHECK_BeliefIDs(qp1, qp3, 3, "c(c3,c6)");
 
  CHECK_BeliefIDs(qp1, qp4, 4, "dprime(c1,c4)");
  CHECK_BeliefIDs(qp1, qp4, 4, "dprime(c2,c5)");
  CHECK_BeliefIDs(qp1, qp4, 4, "dprime(c3,c6)");

  #if 0
  // query plan of ctx 1 must be sufficient to parse bridge rules!
  BridgeRuleTablePtr bridge_rules1 = BridgeRuleParser::parseString(brfile1, qp1);
  #else
  // bridge rules C3,C4 --> C1
  BridgeRuleTablePtr bridge_rules1(new BridgeRuleTable);

  ID id_ac1_11 = at1ctx1.localSignature->getIDByString(str_ac1);
  ID id_ac2_11 = at1ctx1.localSignature->getIDByString(str_ac2);
  ID id_ac3_11 = at1ctx1.localSignature->getIDByString(str_ac3);
  ID id_cc1c4_31 = at1ctx3.groundInputSignature->getIDByString(str_cc1c4);
  ID id_cc2c5_31 = at1ctx3.groundInputSignature->getIDByString(str_cc2c5);
  ID id_cc3c6_31 = at1ctx3.groundInputSignature->getIDByString(str_cc3c6);
  ID id_dprimec1c4_41 = at1ctx4.groundInputSignature->getIDByString(str_dprimec1c4);
  ID id_dprimec2c5_41 = at1ctx4.groundInputSignature->getIDByString(str_dprimec2c5);
  ID id_dprimec3c6_41 = at1ctx4.groundInputSignature->getIDByString(str_dprimec3c6);
  // adding all ground instances of a(X) :- (3:c(X,Y)), not (4:dprime(X,Y)).
  //
  // a(c1) :- (3:c(c1,c4)), not (4:dprime(c1,c4)).
  Tuple body11;
  body11.push_back(id_cc1c4_31);
  body11.push_back(ID::nafLiteralFromBelief(id_dprimec1c4_41));
  BridgeRule br11(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_ac1_11, body11);
  bridge_rules1->storeAndGetID(br11);

  // a(c2) :- (3:c(c2,c5)), not (4:dprime(c2,c5)).
  Tuple body21;
  body21.push_back(id_cc2c5_31);
  body21.push_back(ID::nafLiteralFromBelief(id_dprimec2c5_41));
  BridgeRule br21(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_ac2_11, body21);
  bridge_rules1->storeAndGetID(br21); 

  // a(c3) :- (3:c(c3,c6)), not (4:dprime(c3,c6)).
  Tuple body31;
  body31.push_back(id_cc3c6_31);
  body31.push_back(ID::nafLiteralFromBelief(id_dprimec3c6_41));
  BridgeRule br31(ID::MAINKIND_RULE | ID::SUBKIND_RULE_BRIDGE_RULE, id_ac3_11, body31);
  bridge_rules1->storeAndGetID(br31);
  #endif

  //  #if INSTANTIATE

  #warning what's that offset?
  std::size_t ctx_off31 = 0;
  std::size_t ctx_off41 = 1;
  NewNeighborPtr neighbor31(new NewNeighbor(ctx_id3, ctx_off31, ctx_hostname3, port3));
  NewNeighborPtr neighbor41(new NewNeighbor(ctx_id4, ctx_off41, ctx_hostname4, port4));
  NewNeighborVecPtr neighbors1(new NewNeighborVec);
  neighbors1->push_back(neighbor31);
  neighbors1->push_back(neighbor41);

  NewNeighborVecPtr neighbors_reg1(new NewNeighborVec);
  neighbors_reg1->push_back(neighbor31);
  neighbors_reg1->push_back(neighbor41);

  EnginePtr dlv_engine1 = DLVEngine::create();
  EngineWPtr dlv_engine_wp1(dlv_engine1);
  InstantiatorPtr dlv_inst1 = dlv_engine1->createInstantiator(dlv_engine_wp1, kbspec1);

  #warning for a clean interface, the context should store qp1 and the index 1 (i.e., that it is the context with index 1 in this plan) Note: we could make an autodetection of the index 1: a query plan only contains a localSignature for the context where it is located
  NewBeliefState* interface10(new NewBeliefState(SYSTEM_SIZE, BS_SIZE, true));
  ReturnPlanMapPtr return_plan1(new ReturnPlanMap);
  return_plan1->insert(std::make_pair<std::size_t, NewBeliefState*>(invoker, interface10));

  NewContextPtr ctx1(new NewContext(ctx_id1, pack_size, dlv_inst1, at1ctx1.localSignature, return_plan1, bridge_rules1, neighbors1));
  NewContextVecPtr contexts1(new NewContextVec);
  contexts1->push_back(ctx1);

  RegistryPtr reg1(new Registry(SYSTEM_SIZE, QUEUE_SIZE, BS_SIZE, contexts1, neighbors_reg1));
  boost::thread server_thread1(run_server, ctx_port1, reg1);

  boost::posix_time::milliseconds servers_starting_up(500);
  boost::this_thread::sleep(servers_starting_up);
  //  #endif

  std::size_t query_order1 = 1;
  std::size_t qid1 = query_id(invoker, ctx_id1, query_order1);

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
