#include "network/ConcurrentMessageQueueFactory.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/JoinThread.h"
#include "network/JoinerDispatcher.h"
#include "dmcs/Log.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testJoinLeak"

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;



void send_trigger(ConcurrentMessageQueue* sjn, std::size_t k1, std::size_t k2)
{
  AskNextNotification* notif = new AskNextNotification(BaseNotification::NEXT, 0, 0, k1, k2);
  sjn->send(&notif, sizeof(notif), 0);
}


void send_model(MessagingGatewayBCPtr mg, std::size_t noff, std::size_t offset, std::string input)
{
  std::istringstream iss;
  iss.str(input);
  PartialBeliefState bs;
  iss >> bs;
  PartialBeliefState* pbs = new PartialBeliefState(bs);
  mg->sendModel(pbs, 0, 0, noff, offset, 0); 
}


void send_null(MessagingGatewayBCPtr mg, std::size_t noff, std::size_t offset)
{
  mg->sendModel(0, 0, 0, noff, offset, 0);
}


void send_first_pack_21(MessagingGatewayBCPtr mg, std::size_t noff_2, std::size_t offset_2)
{

  std::string input_21_1 = "{} {0 21} {0 31} {0 41}";
  std::string input_21_2 = "{} {0 22} {0 31} {0 41}";
  std::string input_21_3 = "{} {0 23} {0 31} {0 41}";

  send_model(mg, noff_2, offset_2, input_21_1);
  send_model(mg, noff_2, offset_2, input_21_2);
  send_model(mg, noff_2, offset_2, input_21_3);
  send_null(mg, noff_2, offset_2);

  mg->sendJoinIn(4, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
}



void send_second_pack_21(MessagingGatewayBCPtr mg, std::size_t noff_2, std::size_t offset_2)
{
  std::string input_21_4 = "{} {0 24} {0 31} {0 41}";
  std::string input_21_5 = "{} {0 25} {0 32} {0 42}";
  std::string input_21_6 = "{} {0 25} {0 32} {0 42}";

  send_model(mg, noff_2, offset_2, input_21_4);
  send_model(mg, noff_2, offset_2, input_21_5);
  send_model(mg, noff_2, offset_2, input_21_6);
  send_null(mg, noff_2, offset_2);

  mg->sendJoinIn(4, noff_2, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
}



void send_first_pack_31(MessagingGatewayBCPtr mg, std::size_t noff_3, std::size_t offset_3)
{
  std::string input_31_1 = "{} {} {0 30} {0  41}";
  std::string input_31_2 = "{} {} {0 31} {0  41}";
  std::string input_31_3 = "{} {} {0 32} {0  41}";

  send_model(mg, noff_3, offset_3, input_31_1);
  send_model(mg, noff_3, offset_3, input_31_2);
  send_model(mg, noff_3, offset_3, input_31_3);
  send_null(mg, noff_3, offset_3);

  mg->sendJoinIn(4, noff_3, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
}



void send_second_pack_31(MessagingGatewayBCPtr mg, std::size_t noff_3, std::size_t offset_3)
{
  std::string input_31_4 = "{} {} {0 32} {0 41}";
  std::string input_31_5 = "{} {} {0 33} {0 42}";
  std::string input_31_6 = "{} {} {0 33} {0 42}";

  send_model(mg, noff_3, offset_3, input_31_4);
  send_model(mg, noff_3, offset_3, input_31_5);
  send_model(mg, noff_3, offset_3, input_31_6);
  send_null(mg, noff_3, offset_3);

  mg->sendJoinIn(4, noff_3, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
}



void send_first_pack_41(MessagingGatewayBCPtr mg, std::size_t noff_4, std::size_t offset_4)
{
  std::string input_41_1 = "{} {} {} {0 40}";
  std::string input_41_2 = "{} {} {} {0 41}";
  std::string input_41_3 = "{} {} {} {0 42}";

  send_model(mg, noff_4, offset_4, input_41_1);
  send_model(mg, noff_4, offset_4, input_41_2);
  send_model(mg, noff_4, offset_4, input_41_3);
  send_null(mg, noff_4, offset_4);

  mg->sendJoinIn(4, noff_4, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
}



// End Of Pack
void send_eop(MessagingGatewayBCPtr mg, std::size_t noff, std::size_t offset)
{
  send_null(mg, noff, offset);
  mg->sendJoinIn(1, noff, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
}



BOOST_AUTO_TEST_CASE ( testJoinLeak )
{
  init_loggers("testJoinLeak");

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
  MessagingGatewayBCPtr mg = mqf.createMessagingGateway(port, no_nbs, 20);

  ConcurrentMessageQueueVecPtr jnn(new ConcurrentMessageQueueVec);
  ConcurrentMessageQueuePtr    n0 (new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr    n1 (new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr    n2 (new ConcurrentMessageQueue);
  ConcurrentMessageQueuePtr    jsn (new ConcurrentMessageQueue(5));
  ConcurrentMessageQueuePtr    sjn (new ConcurrentMessageQueue(5));
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

  JoinerDispatcher jd;

  // path = 0, cid = 1, sid = 0
  JoinThread jt(0, 1, 0, &jd);
  boost::thread join_thread(jt, no_nbs, system_size, mg.get(), jsn.get(), sjn.get(), jnn.get());


  std::string from = "Joiner";
  jd.registerThread(0, jt.getCMQ(), from);
  boost::thread join_dispatcher_thread(jd, mg.get());
  
  // simulate incoming messages by putting partial equilibria into JOIN_IN_MQs
  // sleep 1 sec before each round

  send_trigger(sjn.get(), 1, 3);

  send_first_pack_21(mg, noff_2, offset_2);
  send_first_pack_31(mg, noff_3, offset_3);
  send_first_pack_41(mg, noff_4, offset_4);

  for (std::size_t i = 0; i < 3; ++i)
    {
      std::cerr << "I AM HERE. SENDING TRIGGER NUMBER " << i << std::endl;
      send_trigger(sjn.get(), 1, 3);
    }
  std::cerr << "CONTINUE " << std::endl;


  // **************************************
  sleep(1);
  send_second_pack_21(mg, noff_2, offset_2);

  // **************************************

  send_trigger(sjn.get(), 1, 3);
  sleep(1);
  send_eop(mg, noff_2, offset_2);

  // **************************************
  sleep(1);
  send_second_pack_31(mg, noff_3, offset_3);
  
  sleep(1);
  send_first_pack_21(mg, noff_2, offset_2);  

  // **************************************
  sleep(1);
  send_second_pack_21(mg, noff_2, offset_2);

  // **************************************
  sleep(1);
  send_eop(mg, noff_2, offset_2);

  // **************************************
  sleep(1);
  send_eop(mg, noff_3, offset_3);

  // **************************************
  sleep(1);
  send_eop(mg, noff_4, offset_4);

  std::size_t join_count = 0;

  std::cerr << "Finished sending" << std::endl;
  
  while (1)
    {
      std::size_t prio = 0;
      int timeout = 0;

      struct MessagingGatewayBC::ModelSession ms = receive_model(jsn.get());

      if (ms.m == 0)
	{
	  break;
	}
      
      delete ms.m;
      ms.m = 0;

      join_count++;
    }

  std::cerr << "join_count = " << join_count << std::endl;

  //  BOOST_CHECK_EQUAL(join_count, 4);


  join_thread.interrupt();
  join_thread.join();
  join_dispatcher_thread.interrupt();
  join_dispatcher_thread.join();
}
