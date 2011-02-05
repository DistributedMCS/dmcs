#include "network/ConcurrentMessageQueueFactory.h"
#include "network/JoinThread.h"
#include "dmcs/Log.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testJoinThread"

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;

BOOST_AUTO_TEST_CASE ( testJoinThread )
{
  init_loggers("testJoinThread");

  std::size_t no_nbs      = 3;
  std::size_t system_size = 4;
  std::size_t port        = 212409;

  /*
    topology:
                   (1)_______
                  /   \      \
                 /     \      \
                (2)----(3)    /
                 \     /     /
                  \   /     /
                   (4)_____/

    Supppose that pack_size = 3
  */

  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
  MessagingGatewayBCPtr mg = mqf.createMessagingGateway(port, no_nbs, 5);

  ConcurrentMessageQueueVecPtr jnn(new ConcurrentMessageQueueVec);
  ConcurrentMessageQueuePtr    n0 (new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr    n1 (new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr    n2 (new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr    sjn (new ConcurrentMessageQueue);
  ConflictVecPtr    cs (new ConflictVec);

  jnn->push_back(n0);
  jnn->push_back(n1);
  jnn->push_back(n2);

  std::size_t noff_2   = 0;
  std::size_t noff_3   = 1;
  std::size_t noff_4   = 2;

  std::size_t offset_2 = ConcurrentMessageQueueFactory::NEIGHBOR_MQ;
  std::size_t offset_3 = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + 1;
  std::size_t offset_4 = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + 2;

  PartialBeliefState pa;
  PartialBeliefState d;

#warning disabled testcase

#if 0

  JoinThread jt(0xdeadbeef, 42);
  boost::thread t(jt, no_nbs, system_size, mg.get(), jnn.get(), sjn.get(), cs.get(), &pa, &d);

  std::istringstream iss;
  
  // simulate incoming messages by putting partial equilibria into JOIN_IN_MQs
  // sleep 1 sec before each round

  std::string input_21_1 = "{} {0 21} {0 31} {0 41}";
  std::string input_21_2 = "{} {0 22} {0 31} {0 41}";
  std::string input_21_3 = "{} {0 23} {0 31} {0 41}";

  std::string input_21_4 = "{} {0 24} {0 31} {0 41}";
  std::string input_21_5 = "{} {0 25} {0 32} {0 42}";

  std::string input_31_1 = "{} {} {0 30} {0  41}";
  std::string input_31_2 = "{} {} {0 31} {0  41}";
  std::string input_31_3 = "{} {} {0 32} {0  41}";

  std::string input_31_4 = "{} {} {0 32} {0 41}";
  std::string input_31_5 = "{} {} {0 33} {0 42}";

  std::string input_41_1 = "{} {} {} {0 40}";
  std::string input_41_2 = "{} {} {} {0 41}";
  std::string input_41_3 = "{} {} {} {0 42}";

  // *********************************************************************
  iss.str(input_21_1);
  PartialBeliefState  bs_21_1;
  PartialBeliefState* pbs_21_1 = &bs_21_1;
  iss >> bs_21_1;
  mg->sendModel(pbs_21_1, 0, noff_2, offset_2, 0); 

  iss.clear();
  iss.str(input_21_2);
  PartialBeliefState  bs_21_2;
  PartialBeliefState* pbs_21_2 = &bs_21_2;
  iss >> bs_21_2;
  mg->sendModel(pbs_21_2, 0, noff_2, offset_2, 0); 

  mg->sendJoinIn(2, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************
  iss.clear();
  iss.str(input_31_1);
  PartialBeliefState  bs_31_1;
  PartialBeliefState* pbs_31_1 = &bs_31_1;
  iss >> bs_31_1;
  mg->sendModel(pbs_31_1, 0, noff_3, offset_3, 0);

  iss.clear();
  iss.str(input_31_2);
  PartialBeliefState  bs_31_2;
  PartialBeliefState* pbs_31_2 = &bs_31_2;
  iss >> bs_31_2;
  mg->sendModel(pbs_31_2, 0, noff_3, offset_3, 0);

  mg->sendJoinIn(2, noff_3, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************
  iss.clear();
  iss.str(input_41_1);
  PartialBeliefState  bs_41_1;
  PartialBeliefState* pbs_41_1 = &bs_41_1;
  iss >> bs_41_1;
  mg->sendModel(pbs_41_1, 0, noff_4, offset_4, 0);

  iss.clear();
  iss.str(input_41_2);
  PartialBeliefState  bs_41_2;
  PartialBeliefState* pbs_41_2 = &bs_41_2;
  iss >> bs_41_2;
  mg->sendModel(pbs_41_2, 0, noff_4, offset_4, 0);

  mg->sendJoinIn(2, noff_4, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************

  sleep(1);

  iss.clear();
  iss.str(input_21_3);
  PartialBeliefState  bs_21_3;
  PartialBeliefState* pbs_21_3 = &bs_21_3;
  iss >> bs_21_3;
  mg->sendModel(pbs_21_3, 0, noff_2, offset_2, 0);
  mg->sendModel(0,        0, noff_2, offset_2, 0);

  mg->sendJoinIn(2, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************

  iss.clear();
  iss.str(input_31_3);
  PartialBeliefState  bs_31_3;
  PartialBeliefState* pbs_31_3 = &bs_31_3;
  iss >> bs_31_3;
  mg->sendModel(pbs_31_3, 0, noff_3, offset_3, 0);
  mg->sendModel(0,        0, noff_3, offset_3, 0);
  
  mg->sendJoinIn(2, noff_3, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************

  sleep(1);
  
  iss.clear();
  iss.str(input_41_3);
  PartialBeliefState  bs_41_3;
  PartialBeliefState* pbs_41_3 = &bs_41_3;
  iss >> bs_41_3;
  mg->sendModel(pbs_41_3, 0, noff_4, offset_4, 0);
  mg->sendModel(0,        0, noff_4, offset_4, 0);
  
  mg->sendJoinIn(2, noff_4, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
  
  // *********************************************************************
  
  sleep(1);
  
  iss.clear();
  iss.str(input_21_4);
  PartialBeliefState  bs_21_4;
  PartialBeliefState* pbs_21_4 = &bs_21_4;
  iss >> bs_21_4;
  mg->sendModel(pbs_21_4, 0, noff_2, offset_2, 0);
  
  iss.clear();
  iss.str(input_21_5);
  PartialBeliefState  bs_21_5;
  PartialBeliefState* pbs_21_5 = &bs_21_5;
  iss >> bs_21_5;
  mg->sendModel(pbs_21_5, 0, noff_2, offset_2, 0);

  mg->sendModel(0,        0, noff_2, offset_2, 0);

  mg->sendJoinIn(3, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************

  sleep(1);
  mg->sendJoinIn(0, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************

  sleep(1);
  
  iss.clear();
  iss.str(input_31_4);
  PartialBeliefState  bs_31_4;
  PartialBeliefState* pbs_31_4 = &bs_31_4;
  iss >> bs_31_4;
  mg->sendModel(pbs_31_4, 0, noff_3, offset_3, 0);

  iss.clear();
  iss.str(input_31_5);
  PartialBeliefState  bs_31_5;
  PartialBeliefState* pbs_31_5 = &bs_31_5;
  iss >> bs_31_5;
  mg->sendModel(pbs_31_5, 0, noff_3, offset_3, 0);

  mg->sendModel(0,        0, noff_3, offset_3, 0);

  mg->sendJoinIn(3, noff_3, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************
  sleep(1);

  mg->sendModel(pbs_21_1, 0, noff_2, offset_2, 0);
  mg->sendModel(pbs_21_2, 0, noff_2, offset_2, 0);
  mg->sendModel(pbs_21_3, 0, noff_2, offset_2, 0);
  mg->sendModel(0,        0, noff_2, offset_2, 0);
  
  mg->sendJoinIn(4, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  // *********************************************************************
  sleep(1);
  mg->sendModel(pbs_21_4, 0, noff_2, offset_2, 0);
  mg->sendModel(pbs_21_5, 0, noff_2, offset_2, 0);
  mg->sendModel(0,        0, noff_2, offset_2, 0);
  
  mg->sendJoinIn(3, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
  
  // *********************************************************************
  sleep(1);
  mg->sendJoinIn(0, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  sleep(1);
  mg->sendJoinIn(0, noff_3, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  sleep(1);
  mg->sendJoinIn(0, noff_4, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

  std::size_t join_count = 0;
  
  while (1)
    {
      std::size_t prio = 0;
      int timeout       = 0;

      struct MessagingGatewayBC::ModelSession ms = mg->recvModel(ConcurrentMessageQueueFactory::JOIN_OUT_MQ, prio, timeout);

      if (!ms.m == 0)
	{
	  break;
	}
      
      join_count++;
    }

  BOOST_CHECK_EQUAL(join_count, 4);


  t.interrupt();
  t.join();

#endif//0
}
