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

namespace dmcs {

OutputThread::OutputThread(const connection_ptr& c,
			   std::size_t ps,
			   MessagingGatewayBCPtr& m,
			   ConcurrentMessageQueuePtr& hon)
  : conn(c),
    pack_size(ps),
    mg(m),
    handler_output_notif(hon),
    collecting(false)
{
  DMCS_LOG_TRACE(" pack_size = " << pack_size);
}



void
OutputThread::loop(const boost::system::error_code& e)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      PartialBeliefStateVecPtr res(new PartialBeliefStateVec);
      std::string header = "";

      if (!collecting)
	{
	  wait_for_trigger();
	}

      collect_output(res, header);
      write_result(res, header);
    }
  else
    {
      DMCS_LOG_TRACE(": " << e.message());
    }
}



void
OutputThread::wait_for_trigger()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  // wait for Handler to tell me to return some models
  OutputNotification* on = 0;
  void *ptr         = static_cast<void*>(&on);
  unsigned int    p = 0;
  std::size_t recvd = 0;

  DMCS_LOG_TRACE("Wait for message from Handler");

  handler_output_notif->receive(ptr, sizeof(on), recvd, p);

  if (ptr && on)
    {
      pack_size = on->pack_size;
      left_2_send = on->pack_size;
    }
  else
    {
      DMCS_LOG_FATAL("Got null message: " << ptr << " " << on);
      assert(ptr != 0 && on != 0);
    }

  DMCS_LOG_TRACE("Got a message from Handler. pack_size = " << pack_size);
}



void
OutputThread::collect_output(PartialBeliefStateVecPtr& res, std::string& header)
{
  DMCS_LOG_TRACE(" pack_size = " << pack_size << ", left to send = " << left_2_send);

  // Turn this mode on. It's off only when either we collect and send
  // enough pack_size models, or there's no more model to send.
  collecting = true; 
  
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

  header = HEADER_ANS;
}



void
OutputThread::write_result(PartialBeliefStateVecPtr& res, const std::string& header)
{
  DMCS_LOG_TRACE("header = " << header);

  if (header.find(HEADER_ANS) != std::string::npos)
    { // send some models
      conn->async_write(header,
			boost::bind(&OutputThread::write_models, this,
				    boost::asio::placeholders::error, res));
    }
  else
    { // send EOF
      assert (header.find(HEADER_EOF) != std::string::npos);
      conn->async_write(header,
			boost::bind(&OutputThread::loop, this,
				    boost::asio::placeholders::error));
    }

}



void
OutputThread::write_models(const boost::system::error_code& e, PartialBeliefStateVecPtr& res)
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
      std::size_t port                   = ep.port();

      DMCS_LOG_TRACE(" return message == " << return_mess << " on port :" << port);

      conn->async_write(return_mess,
			boost::bind(&OutputThread::loop, this,
				    boost::asio::placeholders::error));
    }
 else
   {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



void
OutputThread::operator()()
{
  DMCS_LOG_TRACE(" Going to call loop()");

  loop(boost::system::error_code());
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
