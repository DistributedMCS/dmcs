#include "SimpleClient.h"

SimpleClient::SimpleClient(boost::asio::io_service& i,
			   boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
  : io_service(i),
    conn(new connection(io_service))
{
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
  conn->socket().async_connect(endpoint,
			      boost::bind(&SimpleClient::send_header, this,
					  boost::asio::placeholders::error,
					  ++endpoint_iterator,
					  conn, 0));
}



void
SimpleClient::send_header(const boost::system::error_code& e,
			  boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			  connection_ptr conn, std::size_t counter)
{
  if (!e)
    {
    }
  else
    {
    }
}
