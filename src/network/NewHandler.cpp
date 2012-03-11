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

#include "network/NewHandler.h"

namespace dmcs {

NewHandler::NewHandler(std::size_t p)
  : port(p)
{ }



NewHandler::~NewHandler()
{
  if (output_thread && output_thread->joinable())
    {
      output_thread->interrupt();
      output_thread->join();
    }

  if (output_thread)
    {
      delete output_thread;
      output_thread = 0;
    }

  delete output_sender;
  output_sender = 0;
}



void
NewHandler::operator()(NewHandlerPtr hdl,
		       connection_ptr conn,
		       NewConcurrentMessageDispatcherPtr md,
		       NewOutputDispatcherPtr od,
		       ForwardMessage* first_mess)
{
  assert (hdl.get() == this);

  std::size_t qid = first_mess->qid;
  std::size_t invoker = invoker_from_qid(qid);
  output_sender = new NewOutputThread(port, invoker);
  output_thread = new boost::thread(*output_sender, conn, md, od);

  boost::system::error_code e;
  handle_read_message(e, hdl, conn, md, od, first_mess);
}


void
NewHandler::handle_read_message(const boost::system::error_code& e,
				NewHandlerPtr hdl,
				connection_ptr conn,
				NewConcurrentMessageDispatcherPtr md,
				NewOutputDispatcherPtr od,
				ForwardMessage* mess)
{
  if (!e)
    {
      int timeout = 0;
      md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, mess, timeout);
      
      if (!is_shutdown(mess->qid))
	{
	  ForwardMessage* next_mess = new ForwardMessage;
	  conn->async_read(*next_mess,
			   boost::bind(&NewHandler::handle_read_message, this,
				       boost::asio::placeholders::error,
				       hdl,
				       conn,
				       md,
				       od,
				       next_mess));
	}
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}



} // namespace dmcs

// Local Variables:
// mode: C++
// End:
