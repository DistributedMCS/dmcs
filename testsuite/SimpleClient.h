#include "network/connection.hpp"

class SimpleClient
{
public:
  SimpleClient(boost::asio::io_service& i,
	       boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

  void
  send_header(const boost::system::error_code& e,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	      connection_ptr conn,
	      std::size_t counter);
private:
  boost::asio::io_service& io_service;
  connection_ptr conn;
};
