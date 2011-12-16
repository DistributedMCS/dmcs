/* DMCS -- Distributed Nonmonotonic Multi-Context Systems.
 * Copyright (C) 2009, 2010 Minh Dao-Tran, Thomas Krennwallner
 * 
 * This file is part of DMCS.
 *
 *  DMCS is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  DMCS is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with DMCS.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   BeliefStateClient.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  16 10:46:21 2011
 * 
 * @brief  
 * 
 * 
 */


#include <sstream>
#include "BeliefStateClient.h"

BeliefStateClient::BeliefStateClient(boost::asio::io_service& i,
				     boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
				     dmcs::NewBeliefState* ws)
  : io_service(i),
    conn(new connection(io_service)),
    want_send(ws)
{
  std::cerr << "CLIENT: constructor()" << std::endl;
  std::cerr << "CLIENT: Want to send: " << want_send << ": " << *want_send << std::endl;

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
      std::cerr << "CLIENT: writing the message..." << std::endl;
      conn->async_write(*want_send,
			boost::bind(&BeliefStateClient::finalize, this,
				    boost::asio::placeholders::error,
				    conn));
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      conn->socket().close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

      conn->socket().async_connect(endpoint,
				   boost::bind(&BeliefStateClient::send_belief_state, this,
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
BeliefStateClient::finalize(const boost::system::error_code& e, connection_ptr conn)
{
  if (!e)
    {
      std::cerr << "CLIENT: finalizing..." << std::endl;
      conn->socket().close();
    }
  else
    {
      std::cerr << "CLIENT: ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



dmcs::NewBeliefState*
BeliefStateClient::bs_sent()
{
  return want_send;
}


// Local Variables:
// mode: C++
// End:
