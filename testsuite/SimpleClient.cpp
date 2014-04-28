#include <sstream>
#include "SimpleClient.h"

SimpleClient::SimpleClient(boost::asio::io_service& i,
			   boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			   std::size_t ws)
  : io_service(i),
    conn(new connection(io_service)),
    sent(0),
    want_send(ws)
{
  std::cerr << "CLIENT: constructor()" << std::endl;

  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
  conn->socket().async_connect(endpoint,
			      boost::bind(&SimpleClient::send_header, this,
					  boost::asio::placeholders::error,
					  ++endpoint_iterator,
					  conn));
}



void
SimpleClient::send_header(const boost::system::error_code& e,
			  boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			  connection_ptr conn)
{
  if (!e)
    {
      if (sent < want_send)
	{
	  std::stringstream out;
	  out << sent;
	  std::string header = "HEADER " + out.str();
	  
	  std::cerr << "CLIENT: Going to send header = " << header << std::endl;
	  
	  sent++;
	  conn->async_write(header,
			    boost::bind(&SimpleClient::send_header, this,
					boost::asio::placeholders::error,
					endpoint_iterator,
					conn));
	}
      else
	{
	  std::cerr << "CLIENT: Send EOF to Server" << std::endl;
	  std::string header = "EOF";

	  sent++;
	  conn->async_write(header,
			    boost::bind(&SimpleClient::finalize, this,
					boost::asio::placeholders::error,
					conn));
	}
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      conn->socket().close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

      conn->socket().async_connect(endpoint,
				   boost::bind(&SimpleClient::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator,
					       conn));
    }
  else
    {
      std::cerr << "CLIENT: ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
SimpleClient::finalize(const boost::system::error_code& e, connection_ptr conn)
{
  if (!e)
    {
      conn->socket().close();
    }
  else
    {
      std::cerr << "CLIENT: ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



std::size_t
SimpleClient::no_sent()
{
  return sent;
}

// Local Variables:
// mode: C++
// End:
