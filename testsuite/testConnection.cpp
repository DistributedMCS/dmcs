#include "network/connection.hpp"
#include "dmcs/Log.h"

#define BOOST_TEST_DYN_LINK
#define BOOST_TEST_MODULE "testConnection"
#include <boost/test/unit_test.hpp>

#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <iostream>

using namespace dmcs;

boost::asio::io_service ios;
boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 6666);
boost::asio::ip::tcp::acceptor acceptor(ios, endpoint);



void
handle_msg(const boost::system::error_code& e,
	   connection_ptr& /* server */,
	   boost::shared_ptr<std::string> msg)
{
  BOOST_CHECK(!e);

  if (!e)
    {
      BOOST_TEST_MESSAGE("Got message: " << *msg);
      BOOST_CHECK_EQUAL(*msg, "Hello world");
    }
  else
    {
      BOOST_TEST_MESSAGE(e.message());
      throw std::runtime_error(e.message());
    }
}




void
handle_accept(const boost::system::error_code& e,
	      connection_ptr& server)
{
  BOOST_TEST_MESSAGE(__PRETTY_FUNCTION__);

  BOOST_CHECK(!e);

  if (!e)
    {
#if 0
      connection_ptr newconn(new connection(ios));

      acceptor.async_accept(server->socket(),
			    boost::bind(&handle_accept,
					boost::asio::placeholders::error,
					server)
			    );
#endif
      
      boost::shared_ptr<std::string> header(new std::string);
      
      // read header to decide what kind of command type to create
      server->async_read(*header,
			 boost::bind(&handle_msg,
				     boost::asio::placeholders::error,
				     server,
				     header)
			 );
    }
  else
    {
      BOOST_TEST_MESSAGE(e.message());
      throw std::runtime_error(e.message());
    }
}


void
finalize(const boost::system::error_code& /* e */)
{ }


void
send_header(const boost::system::error_code& e,
	    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	    connection_ptr client)
{
  BOOST_CHECK(!e);

  if (!e)
    {
      std::string hello = "Hello world";
      client->async_write(hello,
			  boost::bind(&finalize,
				      boost::asio::placeholders::error)
			  );
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      client->socket().close();
      
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      client->socket().async_connect(endpoint,
				     boost::bind(&send_header,
						 boost::asio::placeholders::error,
						 ++endpoint_iterator,
						 client)
				     );
    }
  else
    {
      BOOST_TEST_MESSAGE(e.message());
      throw std::runtime_error(e.message());
    }
}



boost::shared_ptr<boost::thread> t;


BOOST_AUTO_TEST_CASE ( testServer )
{
  init_loggers("testConnection");


  connection_ptr server(new connection(ios));

  acceptor.async_accept(server->socket(),
			boost::bind(&handle_accept,
				    boost::asio::placeholders::error,
				    server)
			);

  t = boost::shared_ptr<boost::thread>(new boost::thread(boost::bind(&boost::asio::io_service::run, &ios)));
}



BOOST_AUTO_TEST_CASE ( testClient )
{
  connection_ptr client(new connection(ios));

  boost::asio::ip::tcp::resolver resolver(ios);
  boost::asio::ip::tcp::resolver::query query(std::string("localhost"), std::string("6666"));
  boost::asio::ip::tcp::resolver::iterator endpoint_it = resolver.resolve(query);

  client->socket().async_connect(*endpoint_it,
				 boost::bind(&send_header,
					     boost::asio::placeholders::error,
					     ++endpoint_it,
					     client)
				 );

  ios.run();

  t->join();
}

// Local Variables:
// mode: C++
// End:
