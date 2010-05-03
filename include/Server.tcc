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

#include "CommandType.h"


namespace dmcs {

template<typename CmdType>
Server<CmdType>::Server(const CmdType& cmd_,
	       boost::asio::io_service& io_service,
	       const boost::asio::ip::tcp::endpoint& endpoint)
  : io_service_(io_service),
    acceptor_(io_service, endpoint),
    cmd(cmd_)
{
  connection_ptr my_connection(new connection(io_service_));

  acceptor_.async_accept(my_connection->socket(),
			 boost::bind(&Server<CmdType>::handle_accept, this,
				     boost::asio::placeholders::error, my_connection));
}


template<typename CmdType>
void
Server<CmdType>::handle_accept(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Server::Creating new connection..." << std::endl;
#endif // DEBUG

      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor_.io_service()));
      acceptor_.async_accept(new_conn->socket(),
			     boost::bind(&Server<CmdType>::handle_accept, this,
					 boost::asio::placeholders::error, new_conn));

      SessionMsgPtr sesh(new SessionMsg(conn));

#if defined(DEBUG)
      std::cerr << "Server:: Reading..." << std::endl;
#endif // DEBUG

      // go to process this message
      conn->async_read(sesh->mess,
		       boost::bind(&Server<CmdType>::handle_next_message, this,
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


template<typename CmdType>
void
Server<CmdType>::handle_next_message(const boost::system::error_code& e, SessionMsgPtr sesh)
{
  if (!e)
    {
#if defined(DEBUG)
      std::cerr << "in handle_next_message with mess =  " << sesh->mess << std::endl;
#endif //DEBUG
      
      // do the local job
      typename CmdType::return_type result = cmd.execute(sesh->mess);
      
#if defined(DEBUG)
      std::cerr << "Sending result back to invoker" << std::endl;
      //std::cerr << "Sending " << std::endl << result <<std::endl;
#endif //DEBUG
	  
      // Send the result to the client. The connection::async_write()
      // function will automatically serialize the data structure for
      // us.
      sesh->conn->async_write(result,
			      boost::bind(&Server<CmdType>::handle_session, this,
					  boost::asio::placeholders::error, sesh));
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_next_message: " << e.message() << std::endl;
#endif
    }
}


template<typename CmdType>
void
Server<CmdType>::handle_session(const boost::system::error_code& e, SessionMsgPtr sesh)
{
  if (!e)
    {

#if defined(DEBUG)
      std::cerr << "in handle sesstion with mess = " << sesh->mess << std::endl;
#endif //DEBUG

      // after processing the message, check whether it's the last
      // one. PrimitiveCommandType and OptCommandType should always
      // return STOP independently from the message, while
      // DynamicCommandType check whether the flag LAST was turned on
      // in the message.
      if (cmd.continues(sesh->mess))
	{
	  std::cerr << "still continue, going to read messages"  << std::endl;
	  handle_read_message(e, sesh);
	}
      else
	{
	  std::cerr << "go to finalize"  << std::endl;
	  handle_finalize(e, sesh);
	}
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_session: " << e.message() << std::endl;
#endif
    }
}



template<typename CmdType>
void
Server<CmdType>::handle_read_message(const boost::system::error_code& e, SessionMsgPtr sesh)
{
  if (!e)
    {

#if defined(DEBUG)
      std::cerr << "in handle read message: " << std::endl;
#endif //DEBUG

      sesh->conn->async_read(sesh->mess,
		       boost::bind(&Server<CmdType>::handle_next_message, this,
				   boost::asio::placeholders::error, sesh)
		       );
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_read_message: " << e.message() << std::endl;
#endif
    }

}


template<typename CmdType>
void
Server<CmdType>::handle_finalize(const boost::system::error_code& e, SessionMsgPtr /* conn */)
{
  // Nothing to do. The socket will be closed automatically when the last
  // reference to the connection object goes away.
#ifdef DEBUG
  std::cerr << "in handle_finalize: " << std::endl;
#endif
  if (e)
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_finalize: " << e.message() << std::endl;
#endif
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
