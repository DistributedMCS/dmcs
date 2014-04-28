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
 * @file   testNeighbors.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Mar  7 9:09:34 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/BackwardMessage.h"
#include "mcs/JoinIn.h"
#include "mcs/QueryID.h"
#include "network/NewNeighborThread.h"
#include "network/NewConcurrentMessageDispatcher.h"

#include "NeighborInServer.h"
#include "NeighborOutServer.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testNeighbors"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>

using namespace dmcs;


void
run_neighbor_in_server(std::size_t server_port)
{
  std::size_t no_blocks = 5;
  std::size_t block_size = 10;

  boost::asio::io_service io_service_server;
  boost::asio::ip::tcp::endpoint endpoint_server(boost::asio::ip::tcp::v4(), server_port);

  NewBeliefState* bs1 = new NewBeliefState(no_blocks, block_size);
  NewBeliefState* bs2 = new NewBeliefState(no_blocks, block_size);

  bs1->set(0);
  bs1->set(10);
  bs1->set(20);

  bs2->set(1);
  bs2->set(15);
  bs2->set(30);
  bs2->set(45);

  std::size_t qid = 12345;

  ReturnedBeliefState* rbs1 = new ReturnedBeliefState(bs1, qid);
  ReturnedBeliefState* rbs2 = new ReturnedBeliefState(bs2, qid);

  ReturnedBeliefStateListPtr output_list(new ReturnedBeliefStateList);
  output_list->push_back(rbs1);
  output_list->push_back(rbs2);

  NeighborInServer s(io_service_server, endpoint_server, output_list);
  io_service_server.run();
}


BOOST_AUTO_TEST_CASE ( testNeighborIn )
{
  std::size_t nid = 3;
  std::size_t noff = 2;
  std::string hostname = "localhost";
  std::string port = "5050";
  std::size_t server_port = 5050;

  std::size_t queue_size = 10;
  std::size_t no_neighbors = 5;

  boost::thread* server_thread = new boost::thread(run_neighbor_in_server, server_port);
  boost::posix_time::milliseconds server_starting_sleep(100);
  boost::this_thread::sleep(server_starting_sleep);

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(queue_size, no_neighbors));
  NewNeighborPtr neighbor(new NewNeighbor(nid, noff, hostname, port));

  NewNeighborThread* neighbor_in = new NewNeighborThread(neighbor);
  boost::thread* nit = new boost::thread(*neighbor_in, md);

  int timeout = 0;
  NewJoinIn* nji = md->receive<NewJoinIn>(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, timeout);

  BOOST_CHECK_EQUAL(nji->neighbor_offset, noff);
  BOOST_CHECK_EQUAL(nji->no_belief_state, 2); // output_list->size();

  ReturnedBeliefState* rbs1 = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, noff, timeout);
  ReturnedBeliefState* rbs2 = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, noff, timeout);

  BOOST_CHECK_EQUAL(rbs1->qid, 12345);
  BOOST_CHECK_EQUAL(rbs2->qid, 12345);

  BOOST_CHECK_EQUAL(rbs1->belief_state->test(0),  NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(rbs1->belief_state->test(10), NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(rbs1->belief_state->test(20), NewBeliefState::DMCS_TRUE);

  BOOST_CHECK_EQUAL(rbs2->belief_state->test(1),  NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(rbs2->belief_state->test(15), NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(rbs2->belief_state->test(30), NewBeliefState::DMCS_TRUE);
  BOOST_CHECK_EQUAL(rbs2->belief_state->test(45), NewBeliefState::DMCS_TRUE);

  std::cerr << "Finished checking!" << std::endl;

  server_thread->join();
  std::cerr << "Server joined!" << std::endl;
  nit->join();
  std::cerr << "NeighborIn joined!" << std::endl;

  delete server_thread;
  server_thread = 0;
  
  delete nit;
  nit = 0;

  std::cerr << "Sleeping for 2 secs to release the port." << std::endl;
  boost::posix_time::milliseconds final_sleep(2000);
  boost::this_thread::sleep(final_sleep);
}


void
run_neighbor_out_server(std::size_t server_port)
{
  boost::asio::io_service io_service_server;
  boost::asio::ip::tcp::endpoint endpoint_server(boost::asio::ip::tcp::v4(), server_port);

  NeighborOutServer s(io_service_server, endpoint_server);
  io_service_server.run();

  ForwardMessage m = s.getMessage();
  BOOST_CHECK_EQUAL(m.qid, 23456);
  BOOST_CHECK_EQUAL(m.k1, 5);
  BOOST_CHECK_EQUAL(m.k2, 10);
}



BOOST_AUTO_TEST_CASE ( testNeighborOut )
{
  std::size_t nid = 3;
  std::size_t noff = 2;
  std::string hostname = "localhost";
  std::string port = "1040";
  std::size_t server_port = 1040;

  std::size_t queue_size = 10;
  std::size_t no_neighbors = 5;

  boost::thread* server_thread = new boost::thread(run_neighbor_out_server, server_port);
  boost::posix_time::milliseconds server_starting_sleep(100);
  boost::this_thread::sleep(server_starting_sleep);

  NewConcurrentMessageDispatcherPtr md(new NewConcurrentMessageDispatcher(queue_size, no_neighbors));
  NewNeighborPtr neighbor(new NewNeighbor(nid, noff, hostname, port));

  // NeighborOut is implicitly run from NeighborThread
  NewNeighborThread* neighbor_in = new NewNeighborThread(neighbor);
  boost::thread* nit = new boost::thread(*neighbor_in, md);

  ForwardMessage* fwd_mess1 = new ForwardMessage(23456, 5, 10);
  ForwardMessage* fwd_mess2 = new ForwardMessage(shutdown_query_id(), 0, 0);

  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_OUT_MQ, noff, fwd_mess1, timeout);
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_OUT_MQ, noff, fwd_mess2, timeout);

  server_thread->join();
  std::cerr << "Server joined!" << std::endl;
  nit->join();
  std::cerr << "NeighborIn joined!" << std::endl;

  delete server_thread;
  server_thread = 0;
  
  delete nit;
  nit = 0;

  std::cerr << "Sleeping for 2 secs to release the port." << std::endl;
  boost::posix_time::milliseconds final_sleep(2000);
  boost::this_thread::sleep(final_sleep);
}


// Local Variables:
// mode: C++
// End:
