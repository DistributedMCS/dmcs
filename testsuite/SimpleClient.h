#include "network/connection.hpp"

class SimpleClient
{
public:
  SimpleClient(boost::asio::io_service& i,
	       boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	       std::size_t ws);

  void
  send_header(const boost::system::error_code& e,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	      connection_ptr conn);

  void
  finalize(const boost::system::error_code& e, connection_ptr conn);

  std::size_t
  no_sent();

private:
  boost::asio::io_service& io_service;
  connection_ptr conn;
  std::size_t sent;
  std::size_t want_send;
};

// Local Variables:
// mode: C++
// End:
