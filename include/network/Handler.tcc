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

#include "dmcs/Log.h"

namespace dmcs {

template<typename CmdType>
Handler<CmdType>::Handler(CmdTypePtr cmd, connection_ptr conn_)
  : conn(conn_)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

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
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("Message = " << sesh->mess);

      // do the local job
      typename CmdType::return_type result = cmd->execute(sesh->mess);
      const std::string& header = HEADER_ANS;
      
      DMCS_LOG_DEBUG("Got local result, now send back to the invoker the header and then the real result!");
      DMCS_LOG_DEBUG("Header = " << header);
      DMCS_LOG_DEBUG("Result = " << result);
	  
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
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


template<typename CmdType>
void
Handler<CmdType>::send_result(typename CmdType::return_type result, 
			      const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      sesh->conn->async_write(result,
			boost::bind(&Handler<CmdType>::handle_session, this,
				    boost::asio::placeholders::error, sesh, cmd));      
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


template<typename CmdType>
void
Handler<CmdType>::send_eof(const boost::system::error_code& e, SessionMsgPtr sesh)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      const std::string& str_eof = HEADER_EOF;
      sesh->conn->async_write(str_eof,
			boost::bind(&Handler<CmdType>::handle_finalize, this,
				    boost::asio::placeholders::error, sesh));      
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename CmdType>
void
Handler<CmdType>::handle_session(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("mess = " << sesh->mess);

      // after processing the message, check whether it's the last
      // one. PrimitiveCommandType and OptCommandType should always
      // return STOP independently from the message, while
      // DynamicCommandType check whether the flag LAST was turned on
      // in the message.
      if (cmd->continues(sesh->mess))
	{
	  DMCS_LOG_DEBUG("Continue, going to read messages");

	  sesh->conn->async_read(sesh->mess,
				 boost::bind(&Handler<CmdType>::do_local_job, this,
					     boost::asio::placeholders::error, sesh, cmd)
				 );
	}
      else
	{
	  DMCS_LOG_DEBUG("Done, sending EOF");

	  send_eof(e, sesh);
	}
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename CmdType>
void
Handler<CmdType>::handle_finalize(const boost::system::error_code& e, SessionMsgPtr /* sesh */)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  // Nothing to do. The socket will be closed automatically when the last
  // reference to the connection object goes away.

  if (e)
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


// specialized methods for streaming dmcs
Handler<StreamingCommandType>::Handler(StreamingCommandTypePtr cmd, connection_ptr conn_)
  : conn(conn_)
{ 
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  SessionMsgPtr sesh(new SessionMsg(conn));

  conn->async_read(sesh->mess,
		   boost::bind(&Handler<StreamingCommandType>::do_local_job, this,
			       boost::asio::placeholders::error, sesh, cmd, true)
		   );
}


void
Handler<StreamingCommandType>::do_local_job(const boost::system::error_code& e, SessionMsgPtr sesh, StreamingCommandTypePtr cmd, bool first_call)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      // get the unique ID from connection for creating a message gateway just for this connection
      boost::asio::ip::tcp::socket& sock = sesh->conn->socket();
      boost::asio::ip::tcp::endpoint ep = sock.remote_endpoint(); 
      std::size_t port = ep.port();

      if (first_call)
	{
	  DMCS_LOG_DEBUG("First and only initialization, creating MessagingGateway");

	  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
	  mg = mqf.createMessagingGateway(port); // we use the port as unique id

	  std::size_t pack_size = sesh->mess.getPackSize();

	  DMCS_LOG_DEBUG("creating output thread, pack_size = " << pack_size);

	  ots = OutputThreadPtr(new OutputThread(conn, pack_size, mg)); 
	  output_thread = new boost::thread(*ots);

	  stmt = StreamingDMCSThreadPtr(new StreamingDMCSThread(cmd));
	  dmcs_thread = new boost::thread(*stmt);
	}

      // write sesh->mess to QueryMessageQueue

      //DMCS_LOG_DEBUG("executing command");

      // need another thread for dmcs here
      //cmd->execute(sesh->mess, port);
  
      sesh->conn->async_read(header,
			     boost::bind(&Handler<StreamingCommandType>::handle_read_header, this,
					 boost::asio::placeholders::error, sesh, cmd, false)
			     );
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


void
Handler<StreamingCommandType>::handle_read_header(const boost::system::error_code& e, SessionMsgPtr sesh, StreamingCommandTypePtr cmd, bool /* first_call */)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      // check header
      if (header.find(HEADER_NEXT) != std::string::npos)
	{
	  // trigger output thread
	}
      else if (header.find(HEADER_REQ_STM_DMCS) != std::string::npos)
	{
	  // restart
	}
      
      sesh->conn->async_read(sesh->mess,
			     boost::bind(&Handler<StreamingCommandType>::do_local_job, this,
					 boost::asio::placeholders::error, sesh, cmd, false)
			     );
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
