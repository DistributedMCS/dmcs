#include "network/MessageQueueFactory.h"
#include "network/ConcurrentMessageQueueFactory.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testMessagingGateway"
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>  
#include <boost/date_time.hpp>
#include <iostream>

using namespace dmcs;

void
worker(boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >& mg)
{  
  boost::posix_time::seconds s(1);
  boost::this_thread::sleep(s);
  std::size_t prio;

  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
  //BOOST_TEST_MESSAGE("worker thread: reading queue and blocking...");
  //mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);
}
   

BOOST_AUTO_TEST_CASE( testMessagingGateway )
{
  boost::shared_ptr<MessagingGateway<BeliefState,Conflict> > mg1 =
    MessageQueueFactory().createMessagingGateway(0, 5);

  ConcurrentMessageQueueFactory& f = ConcurrentMessageQueueFactory::instance();
  boost::shared_ptr<MessagingGateway<BeliefState,Conflict> > mg2 = f.createMessagingGateway(0, 5);

  BeliefState* b1 = (BeliefState*) 0xdeadbeef;
  BeliefState* b2 = 0;
  std::size_t prio = 0;

  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  b2 = mg2->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio);

  BOOST_CHECK_EQUAL(b1, b2);

  // now fill up the queue
  ///@todo we assumed that queue has size 5
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  boost::thread workerThread(worker, mg2);
       
  BOOST_TEST_MESSAGE("waiting for worker thread to read queue...");

  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  BOOST_TEST_MESSAGE("killing worker thread...");
  workerThread.join();  
}
