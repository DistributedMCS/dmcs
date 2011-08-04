#include "SimpleServer.h"

#include <iostream>
#include <boost/thread.hpp>

SimpleServer::SimpleServer(boost::asio::io_service& i,
			   const boost::asio::ip::tcp::endpoint& endpoint)
  : io_service(i),
    acceptor(io_service, endpoint)
{
  connection_ptr my_connection(new connection(io_service));
  
  acceptor.async_accept(my_connection->socket(),
			boost::bind(&SimpleServer::handle_accept, this,
				    boost::asio::placeholders::error, my_connection)
			);
}



void
SimpleServer::handle_accept(const boost::system::error_code& e, connection_ptr conn)
{
  if (!e)
    {
      std::cerr << "SERVER: Create new connection..." << std::endl;
      connection_ptr new_conn(new connection(acceptor.io_service()));

      acceptor.async_accept(new_conn->socket(),
			    boost::bind(&SimpleServer::handle_accept, this,
					boost::asio::placeholders::error, new_conn)
			    );

      std::cerr << "SERVER: Wait for request from SimpleClient" << std::endl;
      boost::shared_ptr<std::string> header(new std::string);

      conn->async_read(*header,
		       boost::bind(&SimpleServer::handle_read_header, this,
				   boost::asio::placeholders::error, conn, header));
    }
  else
    {
      std::cerr << "SERVER: ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
SimpleServer::handle_read_header(const boost::system::error_code& e, 
				 connection_ptr conn,
				 boost::shared_ptr<std::string> header)
{
  if (!e)
    {
      if (header->find("END") != std::string::npos)
	{
	  // block the incomming messages for a while
	  boost::posix_time::milliseconds n(5000);
	  boost::this_thread::sleep(n);

	  conn->async_read(*header,
			   boost::bind(&SimpleServer::handle_read_header, this,
				       boost::asio::placeholders::error, conn, header));
	}
      else
	{
	  // do nothing
	  std::cerr << "SERVER: BAILING OUT" << std::endl;
	}
    }
  else
    {
      std::cerr << "SERVER: ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



// Local Variables:
// mode: C++
// End:
