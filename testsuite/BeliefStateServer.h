#ifndef BELIEF_STATE_SERVER_H
#define BELIEF_STATE_SERVER_H

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
		  dmcs::NewBeliefState* bs);

  dmcs::NewBeliefState*
  bs_received();

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::set<connection_ptr> conn_man;
  dmcs::NewBeliefState* bs;
};

#endif // BELIEF_STATE_SERVER_H

// Local Variables:
// mode: C++
// End:
