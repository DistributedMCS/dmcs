#include "network/connection.hpp"
#include "mcs/NewBeliefState.h"

class BeliefStateServer
{
public:
  BeliefStateServer(boost::asio::io_service& i,
		    const boost::asio::ip::tcp::endpoint& endpoint);

  void
  handle_accept(const boost::system::error_code& e, 
		connection_ptr conn);

  void
  handle_finalize(const boost::system::error_code& e, 
		  connection_ptr conn,
		  NewBeliefState* bs);

  NewBeliefState*
  bs_received();

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::set<connection_ptr> conn_man;
  NewBeliefState* bs;
};

// Local Variables:
// mode: C++
// End:
