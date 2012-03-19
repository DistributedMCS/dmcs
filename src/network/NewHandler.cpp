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
 * @file   NewHandler.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Mar  9 15:36:00 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/Logger.h"
#include "network/NewHandler.h"

namespace dmcs {

NewHandler::NewHandler(std::size_t p)
  : first_round(true),
    port(p)
{ }



NewHandler::~NewHandler()
{
  if (output_thread && output_thread->joinable())
    {
      output_thread->interrupt();
      output_thread->join();
      delete output_thread;
      output_thread = 0;
    }
}



void
NewHandler::startup(NewHandlerPtr handler,
		    connection_ptr conn,
		    NewConcurrentMessageDispatcherPtr md,
		    NewOutputDispatcherPtr od)
{
  assert (this == handler.get());
  DBGLOG(DBG, "NewHandler::startup()");
  ForwardMessage* mess = new ForwardMessage;
  conn->async_read(*mess,
		   boost::bind(&NewHandler::handle_read_message, this,
			       boost::asio::placeholders::error,
			       handler,
			       conn,
			       md,
			       od,
			       mess));  
}



void
NewHandler::handle_read_message(const boost::system::error_code& e,
				NewHandlerPtr handler,
				connection_ptr conn,
				NewConcurrentMessageDispatcherPtr md,
				NewOutputDispatcherPtr od,
				ForwardMessage* mess)
{
  assert (this == handler.get());
  if (!e)
    {
      if (first_round)
	{
	  first_round = false;
	  std::size_t qid = mess->qid;
	  std::size_t invoker = invoker_from_qid(qid);
	  output_sender = NewOutputThreadPtr(new NewOutputThread(port, invoker));
	  NewOutputWrapper output_wrapper;
	  output_thread = new boost::thread(output_wrapper, output_sender, conn, md, od);	  
	}

      DBGLOG(DBG, "NewHandler: Got message = " << *mess);

      int timeout = 0;
      md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, mess, timeout);
      
      boost::shared_ptr<std::string> header(new std::string);
      conn->async_read(*header,
		       boost::bind(&NewHandler::handle_read_header, this,
				   boost::asio::placeholders::error,
				   handler,
				   conn,
				   md,
				   od,
				   header));
    }
  else
    {
      DBGLOG(ERROR, "NewHandler::handle_read_message(): ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}



void
NewHandler::handle_read_header(const boost::system::error_code& e,
			       NewHandlerPtr handler,
			       connection_ptr conn,
			       NewConcurrentMessageDispatcherPtr md,
			       NewOutputDispatcherPtr od,
			       boost::shared_ptr<std::string> header)
{
  assert (this == handler.get());

  if (!e)
    {
      if (header->find(HEADER_REQ_DMCS) != std::string::npos)
	{
	  ForwardMessage* mess = new ForwardMessage;
	  conn->async_read(*mess,
			   boost::bind(&NewHandler::handle_read_message, this,
				       boost::asio::placeholders::error,
				       handler,
				       conn,
				       md,
				       od,
				       mess));
	}
      else
	{
	  assert (header->find(HEADER_TERMINATE) != std::string::npos);
	  ForwardMessage* mess = new ForwardMessage;
	  conn->async_read(*mess,
			   boost::bind(&NewHandler::handle_finalize, this,
				       boost::asio::placeholders::error,
				       handler,
				       conn,
				       md,
				       mess));
	}
    }
  else
    {
      DBGLOG(ERROR, "NewHandler::handle_read_header(): ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}



void
NewHandler::handle_finalize(const boost::system::error_code& e,
			    NewHandlerPtr handler,
			    connection_ptr conn,
			    NewConcurrentMessageDispatcherPtr md,
			    ForwardMessage* mess)
{
  assert (this == handler.get());

  if (!e)
    {
      assert (is_shutdown(mess->qid));
      int timeout = 0;
      md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, mess, timeout);

      DBGLOG(DBG, "NewHandler::handle_finalize: closing connection.");
      conn->socket().close();
    }
  else
    {
      DBGLOG(ERROR, "NewHandler::handle_finalize(): ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
