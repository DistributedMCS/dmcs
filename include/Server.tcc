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
 * @file   Server.tcc
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Mar  2 07:17:31 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "PrimitiveDMCS.h"


namespace dmcs {

template<typename CmdType, typename Message_t>
Server<CmdType,Message_t>::Server(DMCSPtr& d,
	       boost::asio::io_service& io_service,
	       const boost::asio::ip::tcp::endpoint& endpoint)
  : io_service_(io_service),
    acceptor_(io_service, endpoint),
    dmcs(d)
{
  connection_ptr my_connection(new connection(io_service_));

  acceptor_.async_accept(my_connection->socket(),
			 boost::bind(&Server<CmdType,Message_t>::handle_accept, this,
				     boost::asio::placeholders::error, my_connection));
}


template<typename CmdType, typename Message_t>
void
Server<CmdType,Message_t>::handle_accept(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Server::Creating new connection..." << std::endl;
#endif // DEBUG

      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor_.io_service()));
      acceptor_.async_accept(new_conn->socket(),
			     boost::bind(&Server<CmdType, Message_t>::handle_accept, this,
					 boost::asio::placeholders::error, new_conn));

      boost::shared_ptr<Session<Message_t> > sesh(new Session<Message_t>(conn));

#if defined(DEBUG)
      std::cerr << "Server::Read V and hist..." << std::endl;
#endif // DEBUG

      //      handle_session
      conn->async_read(sesh->mess,
		       boost::bind(&Server<CmdType, Message_t>::handle_session, this,
				   boost::asio::placeholders::error, sesh)
		       );
    }
  else
    {
      // An error occurred. Log it and return. Since we are not starting a new
      // accept operation the io_service will run out of work to do and the
      // server will exit.

#ifdef DEBUG
      std::cerr << "handle_accept: " << error.message() << std::endl;
#endif
    }
}

template <typename CmdType,typename Message_t>
/// Handle completion of a write operation.
void
Server<CmdType,Message_t>::handle_finalize(const boost::system::error_code& e, connection_ptr /* conn */)
{
  // Nothing to do. The socket will be closed automatically when the last
  // reference to the connection object goes away.
  if (e)
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_finalize: " << e.message() << std::endl;
#endif
    }
}

template<typename CmdType,typename Message_t>
void
Server<CmdType,Message_t>::handle_session(const boost::system::error_code& e,
					  typename boost::shared_ptr<Session<Message_t> > sesh)
{
  if (!e)
    {
#if defined(DEBUG)
      // Print out the data that was received.
      std::cerr << "Message received: " << sesh->mess << std::endl;
#endif //DEBUG

      CmdType c(*dmcs, sesh->mess);
      BeliefStatesPtr belief_states = c.getBeliefStates();


#if defined(DEBUG)
      std::cerr << "Sending belief states back to invoker" << std::endl;
      std::cerr << "Sending " << std::endl << belief_states <<std::endl;
#endif //DEBUG

      // Successfully accepted a new connection. Send the list of models to the
      // client. The connection::async_write() function will automatically
      // serialize the data structure for us.
      //sesh->conn->async_write(*projected_belief_states,
      //sesh->conn->async_write(*belief_states,
      sesh->conn->async_write(belief_states,
			      boost::bind(&Server<CmdType, Message_t>::handle_finalize, this,
					  boost::asio::placeholders::error, sesh->conn));

#if defined(DEBUG)
      // print statistical caching information
      //std::cerr << "Cache statistics: " << *ctx.getStatsInfo();
#endif //DEBUG
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_session: " << e.message() << std::endl;
#endif
    }

}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
