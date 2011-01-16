#include "network/MessageQueueFactory.h"
#include "network/ConcurrentMessageQueueFactory.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testMessagingGateway"
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp>  
#include <boost/thread/future.hpp>  
#include <boost/date_time.hpp>
#include <iostream>

using namespace dmcs;

void
worker(boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >& mg, boost::promise<int>& p)
{  
  boost::posix_time::seconds s(1);
  boost::this_thread::sleep(s);

  std::size_t prio = 0;
  int timeout = 0;

  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_TEST_MESSAGE("worker thread: reading queue...");
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

  BOOST_TEST_MESSAGE("worker thread: reading queue and blocking for at most 1000msecs...");
  timeout = 1000;
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_CHECK_EQUAL(timeout, 0);

  BOOST_TEST_MESSAGE("worker thread: try reading queue...");
  timeout = -1;
  mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
  BOOST_CHECK_EQUAL(timeout, 0);

  p.set_value(42);
}
   

BOOST_AUTO_TEST_CASE( testMessagingGateway )
{
#if 0
  boost::shared_ptr<MessagingGateway<BeliefState,Conflict> > mg1 =
    MessageQueueFactory().createMessagingGateway(0, 5);
#endif //0

  ConcurrentMessageQueueFactory& f = ConcurrentMessageQueueFactory::instance();
  boost::shared_ptr<MessagingGateway<BeliefState,Conflict> > mg2 = f.createMessagingGateway(0, 5);

  BeliefState* b1 = (BeliefState*) 0xdeadbeef;
  BeliefState* b2 = 0;
  std::size_t prio = 0;
  int timeout = 0;

  BOOST_TEST_MESSAGE("send model...");
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  BOOST_TEST_MESSAGE("recv model...");
  b2 = mg2->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

  BOOST_CHECK_EQUAL(b1, b2);

  // now fill up the queue
  ///@todo we assumed that queue has size 5
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);
  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  BOOST_TEST_MESSAGE("send blocks for 1 sec and eventually fails");

  bool ret = mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio, 1000);

  BOOST_CHECK_EQUAL(ret, false);


  boost::promise<int> promise_a_val;
  boost::unique_future<int> get_future_val = promise_a_val.get_future();
  

  boost::thread workerThread(worker, mg2, boost::ref(promise_a_val));
       
  BOOST_TEST_MESSAGE("waiting for worker thread to read queue...");

  mg2->sendModel(b1, 0, ConcurrentMessageQueueFactory::OUT_MQ, prio);

  BOOST_TEST_MESSAGE("waiting for worker thread to send me a future value...");

  get_future_val.wait();

  BOOST_CHECK_EQUAL(get_future_val.get(), 42);

  BOOST_TEST_MESSAGE("killing worker thread...");
  workerThread.join();  
}
