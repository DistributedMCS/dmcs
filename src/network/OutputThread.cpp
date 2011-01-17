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

OutputThread::OutputThread(const connection_ptr& conn_,
			   std::size_t pack_size_,
			   MessagingGatewayBCPtr& mg_,
			   ConcurrentMessageQueuePtr& hon)
  : conn(conn_),
    pack_size(pack_size_),
    mg(mg_),
    handler_output_notif(hon),
    collecting(false)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " pack_size = " << pack_size);
}



void
OutputThread::loop(const boost::system::error_code& e)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      BeliefStateVecPtr res(new BeliefStateVec);
      std::string       header = "";

      if (!collecting)
	{
	  wait_for_trigger();
	}

      collect_output(res, header);
      write_result(res, header);
    }
  else
    {
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << ": " << e.message());
    }
}



void
OutputThread::wait_for_trigger()
{
  // wait for Handler to tell me to return some models
  OutputNotificationPtr on;
  void *ptr         = static_cast<void*>(&on);
  unsigned int    p = 0;
  std::size_t recvd = 0;

  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Wait for message from Handler");
  handler_output_notif->receive(ptr, sizeof(on), recvd, p);

  pack_size = on->pack_size;

  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Got a message from Handler. pack_size = " << pack_size);
  
  left_2_send = pack_size;
}



void
OutputThread::collect_output(BeliefStateVecPtr& res, std::string& header)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " pack_size = " << pack_size << ", left to send = " << left_2_send);

  // Turn this mode on. It's off only when either we collect and send
  // enough pack_size models, or there's no more model to send.
  collecting = true; 
  
  // be careful with weird value of pack_size. Bug just disappreared
  for (std::size_t i = 1; i <= left_2_send; ++i)
    {
      //DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Read from MQ");
      
      std::size_t prio    = 0;
      int timeout = 200; // milisecs
      BeliefState* bs     = mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
      
      //DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Check result from MQ");
      if (bs == 0) // Ups, a NULL pointer
	{
	  if (timeout == 0)
	    {
	      //DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " TIME OUT");
	      // TIME OUT! Going to send whatever I got so far to the parent
	      if (res->size() > 0)
		{
		  //DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Going to send");
		  header = HEADER_ANS;
		  break;
		}
	      else
		{
		  // decrease counter because we gained nothing so far.
		  --i;
		  //DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Restart, i = " << i);
		  continue;
		}
	    }
	  else 
	    {
	      // either UNSAT of EOF
	      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " NOTHING TO OUTPUT, going to send EOF");

	      // Turn off collecting mode
	      collecting = false; 

	      header = HEADER_EOF;
	      break;
	    }
	}
      
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " got #" << i << ": bs = " << *bs);
      
      // Got something is a reasonable time. Continue collecting.
      res->push_back(bs);
    } // for
}



void
OutputThread::write_result(BeliefStateVecPtr& res, const std::string& header)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

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
OutputThread::write_models(const boost::system::error_code& e, BeliefStateVecPtr& res)
{
  if (!e)
    {
      assert (left_2_send >= res->size());

      left_2_send -= res->size();
      
      StreamingBackwardMessage return_mess(res);
      
      if (left_2_send == 0)
	{
	  collecting = false;
	}

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
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Going to call loop()");

  loop(boost::system::error_code());
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
