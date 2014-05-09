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
 * @file   testBeliefState.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  16 14:16:30 2011
 * 
 * @brief  
 * 
 * 
 */

#include <bm/bm.h>

#include "mcs/BeliefStateOffset.h"
#include "mcs/NewBeliefState.h"
#include "mcs/ReturnedBeliefState.h"
#include "BeliefStateServer.h"
#include "BeliefStateClient.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBeliefState"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <string>

using namespace dmcs;


/****************************************************************************************/

void
run_server(std::size_t server_port, NewBeliefState* bs_sent)
{
  std::cerr << "In Server Thread!" << std::endl;
  boost::asio::io_service io_service_server;
  boost::asio::ip::tcp::endpoint endpoint_server(boost::asio::ip::tcp::v4(), server_port);
  
  BeliefStateServer s(io_service_server, endpoint_server);
  io_service_server.run();

  NewBeliefState* bs_received = s.bs_received();
  std::cerr << "Server Thread: Got belief state = " << bs_received << ": " << *bs_received << std::endl;

  BOOST_CHECK(bs_sent != bs_received);
  BOOST_CHECK_EQUAL(*bs_sent, *bs_received);
  
  delete bs_sent;
  delete bs_received;
  bs_sent = 0;
  bs_received = 0;
}



void
run_client(std::string server_port, NewBeliefState* bs_sent)
{
  std::cerr << "In Client Thread!" << std::endl;

  std::string host_name = "localhost";
  boost::asio::io_service io_service_client;
  boost::asio::ip::tcp::resolver resolver(io_service_client);
  boost::asio::ip::tcp::resolver::query query(host_name, server_port);
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint_client = *it;

  BeliefStateClient c(io_service_client, it, bs_sent);
  io_service_client.run();
}


BOOST_AUTO_TEST_CASE ( testSendingBeliefState )
{
  try
    {
      NewBeliefState* bs_sent = new NewBeliefState(65536);
      bs_sent->set(0);
      bs_sent->set(1000);
      std::cerr << "Client Thread: Want to send: " << bs_sent << ": " << *bs_sent << std::endl;

      NewBeliefState* bs_received;

      std::size_t server_port = 241181;
      boost::thread server_thread(boost::bind(run_server, server_port, bs_sent));

      boost::posix_time::milliseconds n(3000);
      boost::this_thread::sleep(n);

      std::stringstream str_server_port;
      str_server_port << server_port;
      boost::thread client_thread(boost::bind(run_client, str_server_port.str(), bs_sent));
     

      server_thread.join();
      client_thread.join();
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception in testSendingBeliefState: " << e.what() << std::endl;
      std::exit(1);
    }
}

/****************************************************************************************/

BOOST_AUTO_TEST_CASE ( testBeliefStateCombination )
{
  const std::size_t NO_BLOCKS = 3;
  const std::size_t BLOCK_SIZE = 16;

  std::vector<std::size_t> starting_offsets(NO_BLOCKS, 0);

  BeliefStateOffset* bso = BeliefStateOffset::create(NO_BLOCKS, BLOCK_SIZE);

  NewBeliefState s(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState t(NO_BLOCKS * BLOCK_SIZE);

  // setting epsilon bits
  t.set(0);
  s.set(16); t.set(16);
  s.set(32); t.set(32);

  // set values
  t.set(10);
  t.set(15);

  s.set(20); t.set(21);
  s.set(25); t.set(25);

  s.set(35); t.set(35);
  s.set(40); t.set(40);


  bool consistent = combine(s, t, bso->getStartingOffsets(), bso->getMasks());
  BOOST_CHECK_EQUAL(consistent, false);

  s.set(20, NewBeliefState::DMCS_UNDEF);
  s.set(21);

  std::cerr << "s = " << s << std::endl;
  std::cerr << "t = " << t << std::endl;
  
  consistent = combine(s, t, bso->getStartingOffsets(), bso->getMasks());
  BOOST_CHECK_EQUAL(consistent, true);
  std::cerr << "s bowtie t = " << s << std::endl;
}


/****************************************************************************************/

BOOST_AUTO_TEST_CASE ( testRemoveDuplication )
{
  const std::size_t NO_BLOCKS = 3;
  const std::size_t BLOCK_SIZE = 16;

  NewBeliefState* bs1 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs2 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs3 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs4 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs5 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState* bs6 = new NewBeliefState(NO_BLOCKS * BLOCK_SIZE);

  bs1->set(1);
  bs1->set(21);
  bs1->set(40, NewBeliefState::DMCS_FALSE);

  bs2->set(1);
  bs2->set(21);
  bs2->set(40, NewBeliefState::DMCS_FALSE);

  bs3->set(1);
  bs3->set(21);
  bs3->set(40, NewBeliefState::DMCS_FALSE);

  bs4->set(15, NewBeliefState::DMCS_FALSE);
  bs4->set(28);
  bs4->set(42);

  bs5->set(15, NewBeliefState::DMCS_FALSE);
  bs5->set(28);
  bs5->set(42);

  bs6->set(4);
  bs6->set(30);
  bs6->set(40);

  ReturnedBeliefState* rbs1 = new ReturnedBeliefState(bs1, 1);
  ReturnedBeliefState* rbs2 = new ReturnedBeliefState(bs2, 1);
  ReturnedBeliefState* rbs3 = new ReturnedBeliefState(bs3, 2);
  ReturnedBeliefState* rbs4 = new ReturnedBeliefState(bs4, 1);
  ReturnedBeliefState* rbs5 = new ReturnedBeliefState(bs5, 1);
  ReturnedBeliefState* rbs6 = new ReturnedBeliefState(bs6, 1);

  ReturnedBeliefStateListPtr rbsl(new ReturnedBeliefStateList);
  
  rbsl->push_back(rbs1);
  rbsl->push_back(rbs2);
  rbsl->push_back(rbs3);
  rbsl->push_back(rbs4);
  rbsl->push_back(rbs5);
  rbsl->push_back(rbs6);

  std::cerr << "Before removing" << std::endl 
	    << printlist(*rbsl, "", "\n", "") << std::endl;

  rbsl->sort(my_compare);
  remove_duplication(rbsl);

  std::cerr << "After removing" << std::endl 
	    << printlist(*rbsl, "", "\n", "") << std::endl;

  BOOST_CHECK_EQUAL(rbsl->size(), 4);
}

// Local Variables:
// mode: C++
// End:
