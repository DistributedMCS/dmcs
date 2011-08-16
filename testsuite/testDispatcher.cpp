#include "network/OutputDispatcher.h"
#include "network/JoinerDispatcher.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testDispatcher"

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


std::size_t
receive_models(ConcurrentMessageQueue* cmq)
{
  std::size_t count = 0;

  while (1)
    {
      struct MessagingGatewayBC::ModelSession ms = {0, 0, 0};
      std::size_t recvd = 0;
      unsigned int p = 0;
      void* ptr = static_cast<void*>(&ms);

      cmq->receive(ptr, sizeof(ms), recvd, p);
      PartialBeliefState* bs = ms.m;
      BOOST_TEST_MESSAGE("bs = " << bs);
      
      if (bs == 0)
	{
	  BOOST_TEST_MESSAGE("Finish with count = " << count);
	  break;
	}

      BOOST_TEST_MESSAGE("(" << ms.path << " " << ms.sid << ")");

      assert(sizeof(ms) == recvd);
      count++;
    }
  
  return count;
}



BOOST_AUTO_TEST_CASE( testOutputDispatcher )
{
  ConcurrentMessageQueueFactory& f = ConcurrentMessageQueueFactory::instance();
  std::size_t port = 5000;
  std::size_t no_nbs = 2;
  std::size_t mq_size = 20;

  MessagingGatewayBCPtr mg = f.createMessagingGateway(port, no_nbs, mq_size);

  OutputDispatcher od;
  boost::thread* output_dispatcher_thread;

  output_dispatcher_thread = new boost::thread(od, mg.get());

  ConcurrentMessageQueue* cmq1 = new ConcurrentMessageQueue();
  ConcurrentMessageQueue* cmq2 = new ConcurrentMessageQueue();
  ConcurrentMessageQueue* cmq3 = new ConcurrentMessageQueue();

  std::istringstream iss("{0 1} {0 2 3}");
  PartialBeliefState* pbs = new PartialBeliefState();
  iss >> *pbs;

  struct MessagingGatewayBC::ModelSession ms = {pbs, 1, 1};

  od.registerThread(1, cmq1);
  od.registerThread(2, cmq2);
  od.registerThread(3, cmq3);

  std::size_t prio = 0;
  // belief_state, path, session_id, from, to, prio
  mg->sendModel(pbs, 1, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg->sendModel(pbs, 1, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg->sendModel(pbs, 2, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg->sendModel(pbs, 3, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  // send NULL to all cmq? to finish 
  mg->sendModel(0, 1, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg->sendModel(0, 2, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg->sendModel(0, 3, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  // read at all cmq? and check the number of received models
  std::size_t count1 = receive_models(cmq1);
  std::size_t count2 = receive_models(cmq2);
  std::size_t count3 = receive_models(cmq3);

  BOOST_CHECK_EQUAL(count1, 2);
  BOOST_CHECK_EQUAL(count2, 1);
  BOOST_CHECK_EQUAL(count3, 1);

  //boost::posix_time::milliseconds n(500);
  //boost::this_thread::sleep(n);
  output_dispatcher_thread->interrupt();
  output_dispatcher_thread->join();
}



BOOST_AUTO_TEST_CASE ( testJoinerDispatcher )
{
  ConcurrentMessageQueueFactory& f = ConcurrentMessageQueueFactory::instance();
  std::size_t port = 5000;
  std::size_t no_nbs = 2;
  std::size_t mq_size = 20;

  MessagingGatewayBCPtr mg = f.createMessagingGateway(port, no_nbs, mq_size);

  JoinerDispatcher jd;
  boost::thread* joiner_dispatcher_thread;

  joiner_dispatcher_thread = new boost::thread(jd, mg.get());

  ConcurrentMessageQueue* cmq1 = new ConcurrentMessageQueue();
  ConcurrentMessageQueue* cmq2 = new ConcurrentMessageQueue();
  ConcurrentMessageQueue* cmq3 = new ConcurrentMessageQueue();

  
}
