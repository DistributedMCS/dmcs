#ifndef BELIEF_STATE_CLIENT_H
#define BELIEF_STATE_CLIENT_H

#include "network/connection.hpp"
#include "mcs/NewBeliefState.h"

class BeliefStateClient
{
public:
  BeliefStateClient(boost::asio::io_service& i,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		    dmcs::NewBeliefState* ws);

  void
  send_belief_state(const boost::system::error_code& e,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		    connection_ptr conn);

  void
  finalize(const boost::system::error_code& e, connection_ptr conn);

  dmcs::NewBeliefState*
  bs_sent();

private:
  boost::asio::io_service& io_service;
  connection_ptr conn;
  dmcs::NewBeliefState* want_send;
};

#endif // BELIEF_STATE_CLIENT_H

// Local Variables:
// mode: C++
// End:
