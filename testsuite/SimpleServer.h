#include "network/connection.hpp"

#include <boost/shared_ptr.hpp>

class SimpleServer
{
public:
  SimpleServer(boost::asio::io_service& i,
	       const boost::asio::ip::tcp::endpoint& endpoint);

  void
  handle_accept(const boost::system::error_code& e, connection_ptr conn);

  void
  handle_read_header(const boost::system::error_code& e, connection_ptr conn,
		     boost::shared_ptr<std::string> header);

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
};

// Local Variables:
// mode: C++
// End:
