#include "SimpleClient.h"
#include "SimpleServer.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testBlockingAsio"
#include <boost/test/unit_test.hpp>
#include <boost/thread/thread.hpp>
#include <boost/bind.hpp>

std::size_t no_received;
std::size_t no_sent;


void
run_server(std::size_t server_port)
{
  std::cerr << "In server thread!" << std::endl;
  boost::asio::io_service io_service_server;
  boost::asio::ip::tcp::endpoint endpoint_server(boost::asio::ip::tcp::v4(), server_port);
  
  SimpleServer s(io_service_server, endpoint_server);
  io_service_server.run();

  no_received = s.no_received();
}



void
run_client(std::string server_port)
{
  std::string host_name = "localhost";
  boost::asio::io_service io_service_client;
  boost::asio::ip::tcp::resolver resolver(io_service_client);
  boost::asio::ip::tcp::resolver::query query(host_name, server_port);
  boost::asio::ip::tcp::resolver::iterator it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint_client = *it;

  // to see the affect of blocing asio, set want_send to 8000.

  std::size_t want_send = 6;
  SimpleClient c(io_service_client, it, want_send);
  io_service_client.run();

  no_sent = c.no_sent();
  BOOST_CHECK_EQUAL(no_sent, want_send+1);
}




BOOST_AUTO_TEST_CASE ( testBlockingAsio )
{
  try
    {
      std::size_t server_port = 5000;
      boost::thread server_thread(boost::bind(run_server, server_port));

      boost::posix_time::milliseconds n(1000);
      boost::this_thread::sleep(n);

      std::stringstream str_server_port;
      str_server_port << server_port;
      boost::thread client_thread(boost::bind(run_client, str_server_port.str()));
      
      server_thread.join();
      client_thread.join();

      BOOST_CHECK_EQUAL(no_sent, no_received);
    }
  catch (std::exception& e)
    {
      std::cerr << "Exception in testBlockingAsio: " << e.what() << std::endl;
      std::exit(1);
    }
}
