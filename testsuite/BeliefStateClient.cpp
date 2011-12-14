#include <sstream>
#include "BeliefStateClient.h"



BeliefStateClient::BeliefStateClient(boost::asio::io_service& i,
			   boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			   NewBeliefState* ws)
  : io_service(i),
    conn(new connection(io_service)),
    want_send(ws)
{
  std::cerr << "CLIENT: constructor()" << std::endl;

  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
  conn->socket().async_connect(endpoint,
			      boost::bind(&BeliefStateClient::send_belief_state, this,
					  boost::asio::placeholders::error,
					  ++endpoint_iterator,
					  conn));
}



void
BeliefStateClient::send_belief_state(const boost::system::error_code& e,
				     boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
				     connection_ptr conn)
{
  if (!e)
    {
      conn->async_write(header,
			boost::bind(&BeliefStateClient::finalize, this,
				    boost::asio::placeholders::error,
				    endpoint_iterator,
				    conn));
    }
  else
    {
      std::cerr << "CLIENT: ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
BeliefStateClient::finalize(const boost::system::error_code& e, connection_ptr conn)
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



NewBeliefState*
BeliefStateClient::bs_sent()
{
  return want_send;
}


// Local Variables:
// mode: C++
// End:
