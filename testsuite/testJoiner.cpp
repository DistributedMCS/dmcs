#include "mcs/BeliefStateOffset.h"
#include "mcs/ForwardMessage.h"
#include "network/NewConcurrentMessageDispatcher.h"
#include "mcs/StreamingJoiner.h"


#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testJoiner"

#include <boost/test/unit_test.hpp>
#include <boost/thread.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;

void
send_input_belief_state(NewConcurrentMessageDispatcherPtr md, 
			std::size_t neighbor_offset,
			ReturnedBeliefStateList& rbs,
			std::size_t sleeping_time = 0)
{
  boost::posix_time::milliseconds n(sleeping_time);
  boost::this_thread::sleep(n);

  int timeout = 0;

  ForwardMessage* notification = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_OUT_MQ, neighbor_offset, timeout);
  std::size_t qid = notification->query_id;

  std::size_t ctx_id = ctxid_from_qid(qid);
  std::size_t query_order = qorder_from_qid(qid);

  BOOST_CHECK_EQUAL(ctx_id, 5);
  BOOST_CHECK_EQUAL(query_order, 1);

  std::size_t no_rbs = rbs.size();
  NewJoinIn* ji = new NewJoinIn(neighbor_offset, no_rbs);

  md->send<NewJoinIn>(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, ji, timeout);
  
  for (ReturnedBeliefStateList::const_iterator it = rbs.begin(); it != rbs.end(); ++it)
    {
      md->send<ReturnedBeliefState>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_offset, *it, timeout);
    }
}


BOOST_AUTO_TEST_CASE ( testStreamingJoiner )
{
  /*
    topology:
                   (1)_______
                  /   \      \
                 /     \      \
                (2)    (3)    /
                 \     /     /
                  \   /     /
                   (4)_____/

   br1 = {a :- (2:b), not (3:d), (4:g).}
   br2 = {b v c :- (4:g).}
   br3 = {d v e :- (4:f).}
   kb4 = {f v g.}
  */

  const std::size_t NO_BLOCKS = 4;
  const std::size_t BLOCK_SIZE = 2;
  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BLOCKS, BLOCK_SIZE);

  // Initialize input belief states
  // C4:
  // {f, \neg g}
  NewBeliefState* bs41 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs41->setEpsilon(3, bso->getStartingOffsets());
  bs41->set(3, 1, bso->getStartingOffsets());
  bs41->set(3, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  std::size_t qid41 = query_id(5, 1);
  ReturnedBeliefState* rbs41 = new ReturnedBeliefState(bs41, qid41);

  // {\neg f, g}
  NewBeliefState* bs42 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs42->setEpsilon(3, bso->getStartingOffsets());
  bs42->set(3, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs42->set(3, 2, bso->getStartingOffsets());
  std::size_t qid42 = query_id(5, 1);
  ReturnedBeliefState* rbs42 = new ReturnedBeliefState(bs42, qid42);
  ReturnedBeliefState* rbs43 = new ReturnedBeliefState(NULL, qid42);
  ReturnedBeliefStateList rbs4;
  rbs4.push_back(rbs41);
  rbs4.push_back(rbs42);
  rbs4.push_back(rbs43);

  // C3:
  // {d, \neg e}, {f, \neg g}
  NewBeliefState* bs31 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs31->setEpsilon(3, bso->getStartingOffsets());
  bs31->set(3, 1, bso->getStartingOffsets());
  bs31->set(3, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs31->setEpsilon(2, bso->getStartingOffsets());
  bs31->set(2, 1, bso->getStartingOffsets());
  bs31->set(2, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  std::size_t qid31 = query_id(5, 1);
  ReturnedBeliefState* rbs31 = new ReturnedBeliefState(bs31, qid31);

  // {\neg d, e}, {f, \neg g}
  NewBeliefState* bs32 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs32->setEpsilon(3, bso->getStartingOffsets());
  bs32->set(3, 1, bso->getStartingOffsets());
  bs32->set(3, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs32->setEpsilon(2, bso->getStartingOffsets());
  bs32->set(2, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs32->set(2, 2, bso->getStartingOffsets());
  std::size_t qid32 = query_id(5, 1);
  ReturnedBeliefState* rbs32 = new ReturnedBeliefState(bs32, qid32);

  // {\neg d, \neg e}, {f, \neg g}
  NewBeliefState* bs33 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs33->setEpsilon(3, bso->getStartingOffsets());
  bs33->set(3, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs33->set(3, 2, bso->getStartingOffsets());
  bs33->setEpsilon(2, bso->getStartingOffsets());
  bs33->set(2, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs33->set(2, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  std::size_t qid33 = query_id(5, 1);
  ReturnedBeliefState* rbs33 = new ReturnedBeliefState(bs33, qid33);
  ReturnedBeliefState* rbs34 = new ReturnedBeliefState(NULL, qid33);
  ReturnedBeliefStateList rbs3;
  rbs3.push_back(rbs31);
  rbs3.push_back(rbs32);
  rbs3.push_back(rbs34);

  // C2:
  // {b, \neg c}, {\neg f, g}
  NewBeliefState* bs21 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs21->setEpsilon(3, bso->getStartingOffsets());
  bs21->set(3, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs21->set(3, 2, bso->getStartingOffsets());
  bs21->setEpsilon(1, bso->getStartingOffsets());
  bs21->set(1, 1, bso->getStartingOffsets());
  bs21->set(1, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  std::size_t qid21 = query_id(5, 1);
  ReturnedBeliefState* rbs21 = new ReturnedBeliefState(bs21, qid21);

  // {\neg b, c}, {\neg f, g}
  NewBeliefState* bs22 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs22->setEpsilon(3, bso->getStartingOffsets());
  bs22->set(3, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs22->set(3, 2, bso->getStartingOffsets());
  bs22->setEpsilon(1, bso->getStartingOffsets());
  bs22->set(1, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs22->set(1, 2, bso->getStartingOffsets());
  std::size_t qid22 = query_id(5, 1);
  ReturnedBeliefState* rbs22 = new ReturnedBeliefState(bs22, qid22);

  // {\neg b, \neg c}, {f, \neg g}
  NewBeliefState* bs23 = new NewBeliefState(NO_BLOCKS, BLOCK_SIZE);
  bs23->setEpsilon(3, bso->getStartingOffsets());
  bs23->set(3, 1, bso->getStartingOffsets());
  bs23->set(3, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs23->setEpsilon(1, bso->getStartingOffsets());
  bs23->set(1, 1, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  bs23->set(1, 2, bso->getStartingOffsets(), NewBeliefState::DMCS_FALSE);
  std::size_t qid23 = query_id(5, 1);
  ReturnedBeliefState* rbs23 = new ReturnedBeliefState(bs23, qid23);
  ReturnedBeliefState* rbs24 = new ReturnedBeliefState(NULL, qid23);
  ReturnedBeliefStateList rbs2;
  rbs2.push_back(rbs21);
  rbs2.push_back(rbs22);
  rbs2.push_back(rbs24);

  ReturnedBeliefStateList rbs2p;
  rbs2p.push_back(rbs23);
  rbs2p.push_back(rbs24);

  /*************************************************************************************************/

  std::size_t queue_size = 5;
  std::size_t no_neighbors = 3;

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(queue_size, no_neighbors));
  ConcurrentMessageQueuePtr cmq0(new ConcurrentMessageQueue(queue_size));
  ConcurrentMessageQueuePtr cmq1(new ConcurrentMessageQueue(queue_size));
  md->registerMQ(cmq1, NewConcurrentMessageDispatcher::JOIN_IN_MQ, 1);
  md->registerMQ(cmq0, NewConcurrentMessageDispatcher::JOIN_IN_MQ, 0);

  NewJoinerDispatcherPtr joiner_dispatcher(new NewJoinerDispatcher(md));
  std::size_t qid_ctx3 = query_id(3, 1);
  std::size_t qid_ctx5 = query_id(5, 1);
  joiner_dispatcher->registerIdOffset(qid_ctx3, 0);
  joiner_dispatcher->registerIdOffset(qid_ctx5, 1);
  boost::thread joiner_dispatcher_thread(*joiner_dispatcher);

  std::size_t ctx_offset = 1;
  StreamingJoiner streaming_joiner(ctx_offset, no_neighbors, md, joiner_dispatcher);

  // send results to JoinerDispatcher
  boost::thread send_from_4_thread(send_input_belief_state, md, 2, rbs4, 1);
  boost::thread send_from_3_thread(send_input_belief_state, md, 1, rbs3, 1);
  boost::thread send_from_2_thread(send_input_belief_state, md, 0, rbs2, 1);
  boost::thread send_from_2p_thread(send_input_belief_state, md, 0, rbs2p, 2000);

  std::size_t qid = query_id(5, 1);
  std::size_t k1 = 1;
  std::size_t k2 = 10;
  ReturnedBeliefState* res1 = streaming_joiner.trigger_join(qid, k1, k2);

  std::cerr << "res1 = " << *res1 << std::endl;

  joiner_dispatcher_thread.interrupt();
  joiner_dispatcher_thread.join();
  send_from_4_thread.join();
  send_from_3_thread.join();
  send_from_2_thread.join();
  send_from_2p_thread.join();
}
