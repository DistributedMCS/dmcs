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
 * @file   testMessageDispatcher.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Dec 29 17:25:23 2011
 * 
 * @brief  
 * 
 * 
 */


#include "network/NewConcurrentMessageDispatcher.h"
#include "mcs/RequestDispatcher.h"
#include "mcs/NewOutputDispatcher.h"
#include "mcs/ForwardMessage.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testMessageDispatcher"
#include <boost/test/unit_test.hpp>

#include <boost/thread.hpp> 
#include <boost/date_time.hpp>
#include <iostream>

using namespace dmcs;


void
worker(NewConcurrentMessageDispatcher* md)
{  
  boost::posix_time::seconds s(2);
  boost::this_thread::sleep(s);

  ForwardMessage* r1 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, 0, 0);
  ForwardMessage* r2 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, 0, 0);
  ForwardMessage* r3 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, 0, 0);
  ForwardMessage* r4 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, 0, 0);
  ForwardMessage* r5 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, 0, 0);

  std::cerr << "r2 = " << *r2 << std::endl;

  BOOST_CHECK_EQUAL(r2->query_id, 2);
  BOOST_CHECK_EQUAL(r2->k1,  1);
  BOOST_CHECK_EQUAL(r2->k2, 10);
}


BOOST_AUTO_TEST_CASE ( testMessageDispatcher )
{
  ForwardMessage* m1 = new ForwardMessage(1, 1, 10);
  ForwardMessage* m2 = new ForwardMessage(2, 1, 10);
  ForwardMessage* m3 = new ForwardMessage(3, 1, 10);
  ForwardMessage* m4 = new ForwardMessage(4, 1, 10);
  ForwardMessage* m5 = new ForwardMessage(5, 1, 10);
  ForwardMessage* m6 = new ForwardMessage(6, 1, 10);
  std::cerr << "m1 = " << *m1 << std::endl;

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(5, 2));
  
  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, m1, 0);
  ForwardMessage* r1 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, 0);
  std::cerr << "r1 = " << *r1 << std::endl;
  BOOST_CHECK_EQUAL(m1, r1);

  // filling up the queue
  std::size_t neighbor_id = 0;
  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_id, m1, timeout);
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_id, m2, timeout);
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_id, m3, timeout);
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_id, m4, timeout);
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_id, m5, timeout);

  // send block for 1 sec and eventually fail
  bool ret = md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor_id, m6, 1000);
  BOOST_CHECK_EQUAL(ret, false);

  boost::thread workerThread(worker, md.get());
  workerThread.join();
}


/************************************************************************************************************/

void
send_request(const NewConcurrentMessageDispatcherPtr& md)
{
  std::size_t qid1 = query_id(0, 1);
  std::size_t qid2 = query_id(0, 2);
  std::size_t qid3 = query_id(1, 1);
  std::size_t qid4 = query_id(1, 2);

  ForwardMessage* m1 = new ForwardMessage(qid1, 1, 5);
  ForwardMessage* m2 = new ForwardMessage(qid2, 6, 10);
  ForwardMessage* m3 = new ForwardMessage(qid3, 1, 8);
  ForwardMessage* m4 = new ForwardMessage(qid4, 9, 16);


  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, m1, 0);
  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, m2, 0);
  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, m3, 0);
  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, m4, 0);
}



void
receive_request(const NewConcurrentMessageDispatcherPtr& md)
{
  ForwardMessage* m1 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, 0);
  ForwardMessage* m2 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, 0);
  ForwardMessage* m3 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, 1);
  ForwardMessage* m4 = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_MQ, 1);
  
  BOOST_CHECK_EQUAL(m1->k1, 1);
  BOOST_CHECK_EQUAL(m1->k2, 5);
  BOOST_CHECK_EQUAL(m2->k1, 6);
  BOOST_CHECK_EQUAL(m2->k2, 10);

  BOOST_CHECK_EQUAL(m3->k1, 1);
  BOOST_CHECK_EQUAL(m3->k2, 8);
  BOOST_CHECK_EQUAL(m4->k1, 9);
  BOOST_CHECK_EQUAL(m4->k2, 16);
}



void
init_belief_states(NewBeliefState*& bs1,
		   NewBeliefState*& bs2,
		   NewBeliefState*& bs3,
		   NewBeliefState*& bs4)
{
  const std::size_t NO_BLOCKS = 3;
  const std::size_t BLOCK_SIZE = 16;

  NewBeliefState* bs1 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs2 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs3 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs4 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);

  bs1->set(1);
  bs1->set(21);
  bs1->set(40, NewBeliefState::DMCS_FALSE);

  bs2->set(15);
  bs2->set(30, NewBeliefState::DMCS_FALSE);
  bs2->set(42);

  bs3->set(0);
  bs3->set(1);
  bs3->set(47, NewBeliefState::DMCS_FALSE);

  bs4->set(4, NewBeliefState::DMCS_FALSE);
  bs4->set(27);
  bs4->set(23);
}


void
send_output(const NewConcurrentMessageDispatcherPtr& md)
{
  NewBeliefState* bs1;
  NewBeliefState* bs2;
  NewBeliefState* bs3;
  NewBeliefState* bs4;
  init_belief_states(bs1, bs2, bs3, bs4);

  std::size_t qid1 = query_id(0, 1);

  ReturnedBeliefState* rbs1 = new ReturnedBeliefState(bs1, qid1);

  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs1, 0);
  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs2, 0);
  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs3, 0);
  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, rbs4, 0);
}



void
receive_output(const NewConcurrentMessageDispatcherPtr& md)
{
}



BOOST_AUTO_TEST_CASE ( testRequestAndOutputDispatcher )
{
  
}

// Local Variables:
// mode: C++
// End:
