#include "network/connection.hpp"
#include "mcs/NewBeliefState.h"

class BeliefStateClient
{
public:
  BeliefStateClient(boost::asio::io_service& i,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		    NewBeliefState* ws);

  void
  send_belief_state(const boost::system::error_code& e,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		    connection_ptr conn);

  void
  finalize(const boost::system::error_code& e, connection_ptr conn);

  NewBeliefState*
  bs_sent();

private:
  boost::asio::io_service& io_service;
  connection_ptr conn;
  NewBeliefState* want_send;
};

// Local Variables:
// mode: C++
// End:
