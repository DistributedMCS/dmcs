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
 * @file   Handler.tcc
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Sep  29 07:23:24 2010
 * 
 * @brief  
 * 
 * 
 */

#include "network/ThreadFactory.h"

namespace dmcs {

template<typename CmdType>
Handler<CmdType>::Handler(CmdTypePtr cmd, connection_ptr conn_)
  : conn(conn_)
{
#ifdef DEBUG
  std::cerr << "Handler::Handler, going to read message" << std::endl;
#endif

  SessionMsgPtr sesh(new SessionMsg(conn));

  // read and process this message
  conn->async_read(sesh->mess,
		   boost::bind(&Handler<CmdType>::do_local_job, this,
			       boost::asio::placeholders::error, sesh, cmd)
		   );
}



template<typename CmdType>
void
Handler<CmdType>::do_local_job(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {
#if defined(DEBUG)
      std::cerr << "Handler::do_local_job " << std::endl
		<< "Message = " << sesh->mess << std::endl;
#endif //DEBUG

      // do the local job
      typename CmdType::return_type result = cmd->execute(sesh->mess);
      std::string header = HEADER_ANS;
      
#if defined(DEBUG)
      std::cerr << "Got local result, now send back to the invoker the header and then the real result!" << std::endl
		<< "Header = " << header << std::endl
		<< "Result = " << *result <<std::endl;
#endif //DEBUG
	  
      // Send the result to the client. The connection::async_write()
      // function will automatically serialize the data structure for
      // us.

      // first send the invoker some header
      sesh->conn->async_write(header,
			      boost::bind(&Handler<CmdType>::send_result, this, result,
					  boost::asio::placeholders::error, sesh, cmd));
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "Handler::do_local_job: " << e.message() << std::endl;
#endif
    }
}


template<typename CmdType>
void
Handler<CmdType>::send_result(typename CmdType::return_type result, 
			      const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {
 #ifdef DEBUG
      std::cerr << "Handler::send_result" << std::endl;
#endif
      sesh->conn->async_write(result,
			boost::bind(&Handler<CmdType>::handle_session, this,
				    boost::asio::placeholders::error, sesh, cmd));      
    }
  else
    {
#ifdef DEBUG
      std::cerr << "Handler::send_result: " << e.message() << std::endl;
#endif
    }
}


template<typename CmdType>
void
Handler<CmdType>::send_eof(const boost::system::error_code& e, SessionMsgPtr sesh)
{
  if (!e)
    {
 #ifdef DEBUG
      std::cerr << "Handler::send_eof" << std::endl;
#endif
      std::string str_eof = HEADER_EOF;
      sesh->conn->async_write(str_eof,
			boost::bind(&Handler<CmdType>::handle_finalize, this,
				    boost::asio::placeholders::error, sesh));      
    }
  else
    {
#ifdef DEBUG
      std::cerr << "Handler::send_result: " << e.message() << std::endl;
#endif
    }
}



template<typename CmdType>
void
Handler<CmdType>::handle_session(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  if (!e)
    {

#if defined(DEBUG)
      //std::cerr << "in handle session with mess = " << sesh->mess << std::endl;
#endif //DEBUG

      // after processing the message, check whether it's the last
      // one. PrimitiveCommandType and OptCommandType should always
      // return STOP independently from the message, while
      // DynamicCommandType check whether the flag LAST was turned on
      // in the message.
      if (cmd->continues(sesh->mess))
	{
	  std::cerr << "Continue, going to read messages"  << std::endl;
	  sesh->conn->async_read(sesh->mess,
				 boost::bind(&Handler<CmdType>::do_local_job, this,
					     boost::asio::placeholders::error, sesh, cmd)
				 );
	}
      else
	{
	  std::cerr << "Go to send_eof"  << std::endl;
	  send_eof(e, sesh);
	}
    }
  else
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "Handler::handle_session: " << e.message() << std::endl;
#endif
    }
}



template<typename CmdType>
void
Handler<CmdType>::handle_finalize(const boost::system::error_code& e, SessionMsgPtr /* sesh */)
{
  // Nothing to do. The socket will be closed automatically when the last
  // reference to the connection object goes away.
#ifdef DEBUG
  std::cerr << "Handler::handle_finalize: " << std::endl;
#endif
  if (e)
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "Handler::handle_finalize: " << e.message() << std::endl;
#endif
    }
}


// specialized methods for streaming dmcs
Handler<StreamingCommandType>::Handler(StreamingCommandTypePtr cmd, connection_ptr conn_)
  : conn(conn_)
{ 
#ifdef DEBUG
  std::cerr << "Handler<StreamingCommandType>::Handler, going to read message" << std::endl;
#endif
  OutputThreadStarter ots;
  output_thread = new boost::thread(ots);

  // get the unique ID from connection for creating a message gateway just for this connection
  boost::asio::ip::tcp::socket& sock = conn->socket();
  boost::asio::ip::tcp::endpoint ep = sock.remote_endpoint(); 
  port = ep.port();

#ifdef DEBUG
  std::cerr << "Handler<StreamingCommandType>::Handler. port of the connection: " << port << std::endl;
#endif

  mg = MessageQueueFactory().createMessagingGateway(port);

  SessionMsgPtr sesh(new SessionMsg(conn));

  // read and put this message into QueryQueue
  conn->async_read(sesh->mess,
		   boost::bind(&Handler<StreamingCommandType>::do_local_job, this,
			       boost::asio::placeholders::error, sesh, cmd)
		   );
}


void
Handler<StreamingCommandType>::do_local_job(const boost::system::error_code& e, SessionMsgPtr sesh, StreamingCommandTypePtr cmd)
{
  if (!e)
    {
      // write sesh->mess to QueryMessageQueue

#ifdef DEBUG
      std::cerr << "Handler<StreamingCommandType>::do_local_job" << std::endl;
#endif
      cmd->execute(sesh->mess, port);
  
      sesh->conn->async_read(header,
			     boost::bind(&Handler<StreamingCommandType>::handle_read_header, this,
					 boost::asio::placeholders::error, sesh, cmd)
			     );
    }
  else
    {
      // An error occurred.
#ifdef DEBUG
      std::cerr << "Handler::do_local_job: " << e.message() << std::endl;
#endif
    }
}


void
Handler<StreamingCommandType>::handle_read_header(const boost::system::error_code& e, SessionMsgPtr sesh, StreamingCommandTypePtr cmd)
{
  if (!e)
    {
      // check header
      
      sesh->conn->async_read(sesh->mess,
			     boost::bind(&Handler<StreamingCommandType>::do_local_job, this,
					 boost::asio::placeholders::error, sesh, cmd)
			     );
    }
  else
    {
      // An error occurred.
  std::cerr << "header = " << header << std::endl;

#ifdef DEBUG
      std::cerr << "Handler::handle_read_header: " << e.message() << std::endl;
#endif
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
