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

#include "mcs/BeliefState.h"
#include "mcs/NewBeliefState.h"
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


BOOST_AUTO_TEST_CASE ( testBeliefState )
{
  // partial belief states
  std::istringstream iss1("21 25 0 0");
  PartialBeliefState pbs1;
  iss1 >> pbs1;
  BOOST_TEST_MESSAGE("pbs1 = " << pbs1);

  std::istringstream iss2("{0 2 4} {0 3 4} {} {}");
  PartialBeliefState pbs2;
  iss2 >> pbs2;
  BOOST_TEST_MESSAGE("pbs2 = " << pbs2);

  bool pb_equal = (pbs1 == pbs2);
  BOOST_CHECK_EQUAL(pb_equal, true);

  std::istringstream iss3("{-1 2 4} {2 -3 4} {} {}");
  PartialBeliefState pbs3;
  iss3 >> pbs3;
  BOOST_TEST_MESSAGE("pbs3 = " << pbs3);

  // belief states
  std::istringstream iss4("1 9 25 0");
  BeliefStatePtr bs4;
  iss4 >> bs4;
  BOOST_TEST_MESSAGE("bs4  = " << bs4);

  std::istringstream iss5("{0} {0 3} {0 3 4} {}");
  BeliefStatePtr bs5;
  iss5 >> bs5;
  BOOST_TEST_MESSAGE("bs5  = " << bs5);
  bool b_equal = (*bs4 == *bs5);
  BOOST_CHECK_EQUAL(b_equal, true);
}



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

/*
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
}*/

/****************************************************************************************/

BOOST_AUTO_TEST_CASE ( testBitMagicManipulation )
{
  const std::size_t NO_BLOCKS = 3;
  const std::size_t BLOCK_SIZE = 16;

  std::vector<std::size_t> starting_offsets(NO_BLOCKS, 0);

  for (std::size_t i = 1; i < NO_BLOCKS; ++i)
    {
      starting_offsets[i] = starting_offsets[i-1] + BLOCK_SIZE;
    }

  NewBeliefState s(NO_BLOCKS * BLOCK_SIZE);
  NewBeliefState t(NO_BLOCKS * BLOCK_SIZE);
  std::vector<BitMagic> masks(NO_BLOCKS);

  for (std::size_t i = 0; i < NO_BLOCKS; ++i)
    {
      masks[i].resize(NO_BLOCKS * BLOCK_SIZE);
      for (std::size_t j = starting_offsets[i]; j < starting_offsets[i] + BLOCK_SIZE; ++j)
	{
	  masks[i].set_bit(j);
	}
    }
  
  std::cerr << masks[0] << std::endl;
  std::cerr << masks[1] << std::endl;
  std::cerr << masks[2] << std::endl;

  // test comparison

  // setting epsilon bits
  s.set(0);
  s.set(16); t.set(16);
  s.set(32); t.set(32);

  // set values
  s.set(8);
  s.set(10);

  s.set(20); t.set(20);
  s.set(25); t.set(25);

  s.set(35); t.set(35);
  s.set(40); t.set(40);
}
