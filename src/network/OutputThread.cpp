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
			   boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : conn(conn_),
    pack_size(pack_size_),
    mg(mg_)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  DMCS_LOG_DEBUG("pack_size = " << pack_size);
}



void
OutputThread::wait_for_trigger(const boost::system::error_code& e)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  if (!e)
    {
      // wait for boost::condition stuff
      left_2_send = pack_size;
      collect_output(e);
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


void
OutputThread::collect_output(const boost::system::error_code& e)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("pack_size = " << pack_size << ", left to send = " << left_2_send);

      BeliefStateVecPtr res(new BeliefStateVec);
      bool need_send = true;
      
      // be careful with weird value of pack_size. Bug just disappreared
      for (std::size_t i = 0; i < left_2_send; ++i)
	{
	  DMCS_LOG_DEBUG("Read from MQ");

	  std::size_t prio = 0;
	  std::size_t timeout = 100; // milisecs
	  BeliefState* bs = mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);
	  
	  if (bs == 0) // Ups, a NULL pointer
	    {
	      if (timeout == 0)
		{
		  // TIME OUT! Going to send whatever I got so far to the parent
		  if (res->size() > 0)
		    {
		      std::string header = HEADER_ANS;
		      conn->async_write(header,
					boost::bind(&OutputThread::write_result, this,
						    boost::asio::placeholders::error, res)
					);
		      need_send = false;
		      break;
		    }
		  else
		    {
		      // decrease counter because we gained nothing so far.
		      --i;
		    }
		}
	      else 
		{
		  // either UNSAT of EOF
		  DMCS_LOG_DEBUG("NOTHING TO OUTPUT, going to send EOF");
		  break;
		}
	    }
	  
	  DMCS_LOG_DEBUG("got #" << i << ": bs = " << *bs);

	  // Got something is a reasonable time. Continue collecting.
	  res->push_back(bs);
	} // for
      
      std::string header;

      if (need_send)
	{
	  if (res->size() > 0)
	    {
	      header = HEADER_ANS;
	      conn->async_write(header,
				boost::bind(&OutputThread::write_result, this,
					    boost::asio::placeholders::error, res)
				);
	    }
	  else
	    {
	      header = HEADER_EOF;
	      conn->async_write(header,
				boost::bind(&OutputThread::collect_output, this,
					    boost::asio::placeholders::error)
				);
	    }
	}
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


void
OutputThread::write_result(const boost::system::error_code& e, BeliefStateVecPtr res)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      StreamingBackwardMessage return_mess(res);
      
      left_2_send -= res->size();

      conn->async_write(return_mess,
			boost::bind(&OutputThread::collect_output, this,
				    boost::asio::placeholders::error)
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
OutputThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  // check flag from Handler. If he tells me to output then start
  // collecting output from OUT_MQ
  wait_for_trigger(boost::system::error_code());
}




} // namespace dmcs

// Local Variables:
// mode: C++
// End:
