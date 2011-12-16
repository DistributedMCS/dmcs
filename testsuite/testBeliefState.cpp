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

BOOST_AUTO_TEST_CASE ( testSendingBeliefState )
{
  try
    {
      NewBeliefState* bs_sent = new NewBeliefState(65536);
      set(*bs_sent, 0);
      set(*bs_sent, 1000);
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
