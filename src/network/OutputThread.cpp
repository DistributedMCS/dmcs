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
 * @file   OutputThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:30:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/StreamingForwardMessage.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "network/OutputThread.h"

#include "dmcs/Log.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>


namespace dmcs {

  OutputThread::OutputThread()
  { }


OutputThread::~OutputThread()
{
  DMCS_LOG_TRACE("So long, suckers. I'm out.");
}


void
OutputThread::loop(const boost::system::error_code& e,
		   connection_ptr conn,
		   MessagingGatewayBC* mg,
		   ConcurrentMessageQueue* handler_output_notif)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      PartialBeliefStateVecPtr res(new PartialBeliefStateVec);
      std::string header = "";

      if (!collecting)
	{
	  if (!wait_for_trigger(handler_output_notif))
	    {
	      return; // shutdown received
	    }
	}
      
      collect_output(mg, res, header);
      write_result(conn, mg, handler_output_notif, res, header);
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


bool
OutputThread::wait_for_trigger(ConcurrentMessageQueue* handler_output_notif)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  bool retval = true;

  // wait for Handler to tell me to return some models
  OutputNotification* on = 0;
  void *ptr         = static_cast<void*>(&on);
  unsigned int    p = 0;
  std::size_t recvd = 0;

  DMCS_LOG_TRACE("Wait for message from Handler");

  handler_output_notif->receive(ptr, sizeof(on), recvd, p);

  if (ptr && on)
    {
      if (on->type == OutputNotification::REQUEST)
	{
	  pack_size = on->pack_size;
	  left_2_send = on->pack_size;
	  retval = true;
	}
      else if (on->type == OutputNotification::SHUTDOWN)
	{
	  retval = false;
	}

      assert (on->type == OutputNotification::REQUEST || on->type == OutputNotification::SHUTDOWN);
    }
  else
    {
      DMCS_LOG_FATAL("Got null message: " << ptr << " " << on);
      assert(ptr != 0 && on != 0);
    }

  DMCS_LOG_TRACE("Got a message from Handler. pack_size = " << pack_size);

  return retval;
}



void
OutputThread::collect_output(MessagingGatewayBC* mg,
			     PartialBeliefStateVecPtr& res,
			     std::string& header)
{
  DMCS_LOG_TRACE("pack_size = " << pack_size << ", left to send = " << left_2_send);

  // Turn this mode on. It's off only when either we collect and send
  // enough pack_size models, or there's no more model to send.
  collecting = true; 
  header = HEADER_ANS;
  
  // be careful with weird value of pack_size. Bug just disappreared
  for (std::size_t i = 1; i <= left_2_send; ++i)
    {
      //DMCS_LOG_TRACE(" Read from MQ");
      
      std::size_t prio       = 0;
      int timeout            = 200; // milisecs
      PartialBeliefState* bs = mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
      
      //DMCS_LOG_TRACE(" Check result from MQ");
      if (bs == 0) // Ups, a NULL pointer
	{
	  if (timeout == 0)
	    {
	      //DMCS_LOG_TRACE(" TIME OUT");
	      // TIME OUT! Going to send whatever I got so far to the parent
	      if (res->size() > 0)
		{
		  DMCS_LOG_TRACE(" Going to send");
		  header = HEADER_ANS;
		  break;
		}
	      else
		{
		  // decrease counter because we gained nothing so far.
		  --i;
		  //DMCS_LOG_TRACE(" Restart, i = " << i);
		  continue;
		}
	    }
	  else 
	    {
	      // either UNSAT or EOF
	      DMCS_LOG_TRACE(" NOTHING TO OUTPUT, going to send EOF");

	      // Turn off collecting mode
	      collecting = false; 

	      header = HEADER_EOF;
	      break;
	    }
	}
      
      DMCS_LOG_TRACE(" got #" << i << ": bs = " << *bs);
      
      // Got something is a reasonable time. Continue collecting.
      res->push_back(bs);
    } // for
}



void
OutputThread::write_result(connection_ptr conn,
			   MessagingGatewayBC* mg,
			   ConcurrentMessageQueue* handler_output_notif,
			   PartialBeliefStateVecPtr& res,
			   const std::string& header)
{
  DMCS_LOG_TRACE("header = " << header);

  if (header.find(HEADER_ANS) != std::string::npos)
    {
      // send some models
      conn->async_write(header,
			boost::bind(&OutputThread::handle_written_header, this,
				    boost::asio::placeholders::error,
				    conn,
				    mg,
				    handler_output_notif,
				    res)
			);
    }
  else // if (header.find(HEADER_EOF) == std::string::npos)
    {
      assert (header.find(HEADER_EOF) != std::string::npos);

      // send EOF
      conn->async_write(header,
			boost::bind(&OutputThread::loop, this,
				    boost::asio::placeholders::error,
				    conn,
				    mg,
				    handler_output_notif)
			);
    }

  //  assert(false && "header is neither HEADER_ANS nor HEADER_EOF");
}



void
OutputThread::handle_written_header(const boost::system::error_code& e,
				    connection_ptr conn,
				    MessagingGatewayBC* mg,
				    ConcurrentMessageQueue* handler_output_notif,
				    PartialBeliefStateVecPtr& res)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  if (!e)
    {
      assert (left_2_send >= res->size());
      
      left_2_send -= res->size();
      
      // mark end of package by a NULL model
      if (left_2_send == 0)
	{
	  PartialBeliefState* bs = 0;
	  res->push_back(bs);
	  collecting = false;
	}
      
      StreamingBackwardMessage return_mess(res);
      
      boost::asio::ip::tcp::socket& sock = conn->socket();
      boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
      
      DMCS_LOG_TRACE("return message to port " << ep.port());
      DMCS_LOG_TRACE(return_mess);
      
      conn->async_write(return_mess,
			boost::bind(&OutputThread::loop, this,
				    boost::asio::placeholders::error,
				    conn,
				    mg,
				    handler_output_notif)
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
OutputThread::operator()(connection_ptr c,
			 std::size_t ps,
			 MessagingGatewayBC* m,
			 ConcurrentMessageQueue* hon)
{
  pack_size = ps;
  collecting = false;

  //  conn = c;
  //  mg = m;
  //  handler_output_notif = hon;

  DMCS_LOG_TRACE("Going to call loop()");

  loop(boost::system::error_code(), c, m, hon);
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
