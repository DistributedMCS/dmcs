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
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
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
Handler<CmdType>::Handler()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
}


template<typename CmdType>
void
Handler<CmdType>::do_local_job(const boost::system::error_code& e,
			       typename BaseHandler<CmdType>::HandlerPtr hdl,
			       typename BaseHandler<CmdType>::SessionMsgPtr sesh,
			       typename BaseHandler<CmdType>::CmdTypePtr cmd,
			       bool /* first_call */)
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
			      boost::bind(&Handler<CmdType>::send_result, this,
					  boost::asio::placeholders::error,
					  result,
					  hdl,
					  sesh,
					  cmd)
			      );
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
Handler<CmdType>::send_result(const boost::system::error_code& e,
			      typename CmdType::return_type result, 
			      typename BaseHandler<CmdType>::HandlerPtr hdl,
			      typename BaseHandler<CmdType>::SessionMsgPtr sesh,
			      typename BaseHandler<CmdType>::CmdTypePtr cmd)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      sesh->conn->async_write(result,
			      boost::bind(&Handler<CmdType>::handle_session, this,
					  boost::asio::placeholders::error,
					  hdl,
					  sesh,
					  cmd)
			      );      
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
Handler<CmdType>::send_eof(const boost::system::error_code& e,
			   typename BaseHandler<CmdType>::HandlerPtr hdl,
			   typename BaseHandler<CmdType>::SessionMsgPtr sesh)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      const std::string& str_eof = HEADER_EOF;
      sesh->conn->async_write(str_eof,
			      boost::bind(&Handler<CmdType>::handle_finalize, this,
					  boost::asio::placeholders::error,
					  hdl,
					  sesh)
			      );      
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
Handler<CmdType>::handle_session(const boost::system::error_code& e,
				 typename BaseHandler<CmdType>::HandlerPtr hdl,
				 typename BaseHandler<CmdType>::SessionMsgPtr sesh,
				 typename BaseHandler<CmdType>::CmdTypePtr cmd)
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
					     boost::asio::placeholders::error,
					     hdl,
					     sesh,
					     cmd,
					     false) // subsequent call
				 );
	}
      else
	{
	  DMCS_LOG_DEBUG("Done, sending EOF");

	  send_eof(e, hdl, sesh);
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
Handler<CmdType>::handle_finalize(const boost::system::error_code& e,
				  typename BaseHandler<CmdType>::HandlerPtr /* hdl */,
				  typename BaseHandler<CmdType>::SessionMsgPtr /* sesh */)
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



// *********************************************************************************************************************
// Specialized methods for streaming dmcs
Handler<StreamingCommandType>::Handler()
  : handler_dmcs_notif(new ConcurrentMessageQueue),
    handler_output_notif(new ConcurrentMessageQueue)
{ 
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
}


Handler<StreamingCommandType>::~Handler()
{ 
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  DMCS_LOG_TRACE("Send SHUTHDOWN and join Output thread");

  OutputNotification* mess_output = new OutputNotification(0, OutputNotification::SHUTDOWN);
  OutputNotification* ow_output = 
    (OutputNotification*) overwrite_send(handler_output_notif, &mess_output, sizeof(mess_output), 0);
  
  if (ow_output)
    {
      delete ow_output;
      ow_output = 0;
    }
  
  output_thread->join();

  DMCS_LOG_TRACE("Send SHUTDOWN and join DMCS thread");
  
  StreamingDMCSNotification* mess_dmcs = new StreamingDMCSNotification(0,0,0,StreamingDMCSNotification::SHUTDOWN);
  StreamingDMCSNotification* ow_dmcs = 
    (StreamingDMCSNotification*) overwrite_send(handler_dmcs_notif, &mess_dmcs, sizeof(mess_dmcs), 0);

  if (ow_dmcs)
    {
      delete ow_dmcs;
      ow_dmcs = 0;
    }

  streaming_dmcs_thread->join();

  DMCS_LOG_TRACE("Cleanup threads and messages");

  delete mess_output;
  delete mess_dmcs;

  delete output_thread;
  delete streaming_dmcs_thread;

  DMCS_LOG_TRACE("So long, and thanks for all the fish.");
}


void
Handler<StreamingCommandType>::do_local_job(const boost::system::error_code& e,
					    HandlerPtr hdl,
					    SessionMsgPtr sesh,
					    CmdTypePtr cmd,
					    bool first_call)
{
  assert(this == hdl.get());

  if (!e)
    {
      // get the unique ID from the port of the connection for creating a unique message gateway just for this session
      boost::asio::ip::tcp::socket& sock = sesh->conn->socket();
      boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
      std::size_t port                   = ep.port();

      std::size_t invoker                = sesh->mess.getInvoker();
      std::size_t pack_size              = sesh->mess.getPackSize();

      if (first_call)
	{
	  DMCS_LOG_TRACE("First and only initialization, creating MessagingGateway");

	  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
	  mg = mqf.createMessagingGateway(port); // we use the port as unique id

	  DMCS_LOG_TRACE("creating dmcs thread");

	  StreamingDMCSThread stdt;
	  streaming_dmcs_thread = new boost::thread(stdt, cmd.get(), handler_dmcs_notif.get());

	  DMCS_LOG_TRACE("creating output thread, pack_size = " << pack_size);

	  // create MessageQueue between Handler and OutputThread to
	  // inform OutputThread about new incoming message (request
	  // the next pack_size models)

	  OutputThread ot;
	  output_thread = new boost::thread(ot, sesh->conn, pack_size, mg.get(), handler_output_notif.get(), invoker);

	  first_call = false;
	}

      DMCS_LOG_TRACE("Notify my slaves of the new message");

      // notify StreamingDMCS
      DMCS_LOG_TRACE("invoker   = " << invoker);
      DMCS_LOG_TRACE("pack_size = " << pack_size);
      DMCS_LOG_TRACE("port      = " << port);

      StreamingDMCSNotification* mess_dmcs = new StreamingDMCSNotification(invoker, pack_size, port);
      StreamingDMCSNotification* ow_dmcs = 
	(StreamingDMCSNotification*) overwrite_send(handler_dmcs_notif, &mess_dmcs, sizeof(mess_dmcs), 0);

      if (ow_dmcs)
	{
	  delete ow_dmcs;
	  ow_dmcs = 0;
	}

      // notify OutputThread
      OutputNotification* mess_output = new OutputNotification(pack_size);
      OutputNotification* ow_output = 
	(OutputNotification*) overwrite_send(handler_output_notif, &mess_output, sizeof(mess_output), 0);

      if (ow_output)
	{
	  delete ow_output;
	  ow_output = 0;
	}

      // back to waiting for incoming message

      DMCS_LOG_TRACE("Waiting for incoming message from " << invoker << " at " << port << "(" << first_call << ")");

      boost::shared_ptr<std::string> header(new std::string);

      sesh->conn->async_read(*header,
			     boost::bind(&Handler<StreamingCommandType>::handle_read_header, this,
					 boost::asio::placeholders::error,
					 hdl,
					 sesh, 
					 cmd,
					 header,
					 pack_size)
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
Handler<StreamingCommandType>::handle_read_header(const boost::system::error_code& e,
						  HandlerPtr hdl,
						  SessionMsgPtr sesh,
						  CmdTypePtr cmd,
						  boost::shared_ptr<std::string> header,
						  std::size_t pack_size)
{
  assert(this == hdl.get());

  DMCS_LOG_TRACE("Header == " << *header);

  if (!e)
    {
      // Check header
      if (header->find(HEADER_REQ_STM_DMCS) != std::string::npos)
	{
	  DMCS_LOG_TRACE("Got a fresh request. Will inform my slaves about this.");

	  // Read the message to get pack_size and conflict, so that we can inform our slaves
	  sesh->conn->async_read(sesh->mess,
				 boost::bind(&Handler<StreamingCommandType>::do_local_job, this,
					     boost::asio::placeholders::error,
					     hdl,
					     sesh,
					     cmd,
					     false) // subsequent call to local job
				 );
	}
      else if (header->find(HEADER_NEXT) != std::string::npos)
	{
	  // code duplication with part of do_local_job. Just leave it like this for now

	  // notify OutputThread
	  OutputNotification* mess_output = new OutputNotification(pack_size);
	  OutputNotification* ow_output = 
	    (OutputNotification*) overwrite_send(handler_output_notif, &mess_output, sizeof(mess_output), 0);
	  
	  if (ow_output)
	    {
	      delete ow_output;
	      ow_output = 0;
	    }

	  // back to waiting for incoming message
	  
	  DMCS_LOG_TRACE("Back to waiting for incoming message");
	  
	  boost::shared_ptr<std::string> header(new std::string);
	  
	  sesh->conn->async_read(*header,
				 boost::bind(&Handler<StreamingCommandType>::handle_read_header, this,
					     boost::asio::placeholders::error,
					     hdl,
					     sesh, 
					     cmd,
					     header,
					     pack_size)
				 );	  
	}
      else if (header->find(HEADER_TERMINATE) != std::string::npos)
	{
	  // don't do anything, session will disappear

	  DMCS_LOG_TRACE("Closing session with context " << sesh->mess.getInvoker());
	}
      else 
	{
	  DMCS_LOG_ERROR("Got a crappy header: " << *header << ". Back to waiting for the next header.");
	  sesh->conn->async_read(*header,
				 boost::bind(&Handler<StreamingCommandType>::handle_read_header, this,
					     boost::asio::placeholders::error,
					     hdl,
					     sesh,
					     cmd,
					     header,
					     pack_size)
				 );
	}
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
