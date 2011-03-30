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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include "dmcs/StreamingForwardMessage.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "network/OutputThread.h"

#include "dmcs/Log.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>



namespace dmcs {

OutputThread::OutputThread(std::size_t p, std::size_t pa)
  : port(p), 
    path(pa),
    output_buffer(new PartialBeliefStateBuf(CIRCULAR_BUF_SIZE))
{ }



OutputThread::~OutputThread()
{
  DMCS_LOG_TRACE(port << ": Terminating OutputThread.");
  for (PartialBeliefStateBuf::iterator it = output_buffer->begin(); 
       it != output_buffer->end(); ++it)
    {
      assert (*it);
      delete *it;
      *it = 0;
    }
  output_buffer->clear();
}



void
OutputThread::operator()(connection_ptr c,
			 MessagingGatewayBC* mg,
			 ConcurrentMessageQueue* hon)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  //  std::size_t path;
  std::size_t k1;
  std::size_t k2;
  std::size_t parent_session_id;
  ModelSessionIdListPtr res(new ModelSessionIdList);
  
  while (1)
    {
      res->clear();

      //BaseNotification::NotificationType nt = wait_for_trigger(hon, path, k1, k2, parent_session_id);
      BaseNotification::NotificationType nt = wait_for_trigger(hon, k1, k2, parent_session_id);

      if (nt == BaseNotification::SHUTDOWN)
	{
	  return; // shutdown received
	}
      else 
	{
	  assert (nt == BaseNotification::REQUEST || nt == BaseNotification::NEXT);

	  collect_output(mg, res, k1, k2, parent_session_id);

	  DMCS_LOG_TRACE(port << ": Output collected. ");
	  DMCS_LOG_TRACE(port << ": res->size() = " << res->size());

	  if (res->size() > 0)
	    {
	      write_result(c, res);
	    }
	  else
	    {
	      DMCS_LOG_TRACE(port << ": No more answer to send. Send EOF");
	      const std::string header_eof = HEADER_EOF;
	      c->write(header_eof);
	    }
	}
    }
}



BaseNotification::NotificationType
OutputThread::wait_for_trigger(ConcurrentMessageQueue* handler_output_notif,
			       std::size_t& k1,
			       std::size_t& k2,
			       std::size_t& parent_session_id)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  // wait for Handler to tell me to return some models
  OutputNotification* on = 0;
  void *ptr         = static_cast<void*>(&on);
  unsigned int    p = 0;
  std::size_t recvd = 0;
  BaseNotification::NotificationType nt;

  DMCS_LOG_TRACE(port << ": Wait for message from Handler");

  handler_output_notif->receive(ptr, sizeof(on), recvd, p);

  if (ptr && on)
    {
      nt = on->type;
      if (nt == OutputNotification::SHUTDOWN)
	{
	  DMCS_LOG_TRACE(port << ": Got SHUTDOWN from Handler.");
	}
      else
	{
	  path = on->path;
	  k1 = on->k1;
	  k2 = on->k2;
	  parent_session_id = on->parent_session_id;

	  assert (k1 <= k2);

	  assert ((nt == OutputNotification::REQUEST) || (nt == OutputNotification::NEXT));

	  /*if (nt == OutputNotification::REQUEST)
	    {
	      // clean up output buffer because this is a fresh request
	      for (PartialBeliefStateBuf::const_iterator it = output_buffer->begin(); it != output_buffer->end(); ++it)
		{
		  delete *it;
		}
		output_buffer->clear();
		}*/

	  DMCS_LOG_TRACE(port << ": Got a message from Handler. on = " << *on);
	}

      delete on;
      on = 0;
    }
  else
    {
      DMCS_LOG_FATAL("Got null message: " << ptr << " " << on);
      assert(ptr != 0 && on != 0);
    }

  return nt;
}



void
OutputThread::collect_output(MessagingGatewayBC* mg,
			     ModelSessionIdListPtr& res,
			     std::size_t k1,
			     std::size_t k2,
			     std::size_t parent_session_id)
{
  // TOO BAD we cannot cache here!!!
  DMCS_LOG_TRACE(port << ": Going to collect output. k1 = " << k1 << ", k2 = " << k2);

  bool reached_k1 = true;
  res->clear();

  // throw away models up to k1-1 ============================================================================
  for (std::size_t i = 1; i < k1; ++i)
    {
      DMCS_LOG_TRACE(port << ": Read from MQ. i = " << i);
      
      std::size_t prio = 0;
      int timeout = 0;

      struct MessagingGatewayBC::ModelSession ms =
	mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

      PartialBeliefState* bs = ms.m;

      if (bs == 0) // EOF
	{
	  DMCS_LOG_TRACE(port << ": Got EOF before reaching " << k1 << " models. Bailing out...");
	  reached_k1 = false;
	  break;
	}
      else
	{
	  DMCS_LOG_TRACE(port << ": Extract information from MQ");

	  std::size_t pa = ms.path;
	  std::size_t sid = ms.sid;
	  
	  assert (pa == path);

	  DMCS_LOG_TRACE(port << ": got #" << i);
	  DMCS_LOG_TRACE(port << ": bs = " << *bs << "path = " << pa << ", sid = " << sid << ". Notice: parent_session_id = " << parent_session_id);
	  delete bs;
	  bs = 0;
	}
    } // for

  if (!reached_k1)
    {
      return;
    }

  DMCS_LOG_TRACE(port << ": Collecting, k1 = " << k1 << ",  k2 = " << k2);
  // now collect up to k2-k1+1 unique models to return ========================================================
  for (std::size_t i = k1; i <= k2+1; ++i)
    {
      DMCS_LOG_TRACE(port << ": Collecting, i = " << i);
      std::size_t prio = 0;
      int timeout = 0;

      struct MessagingGatewayBC::ModelSession ms =
	mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

      PartialBeliefState* bs = ms.m;

      if (i == k2+1)
	{
	  assert (bs == 0);
	  DMCS_LOG_TRACE(port << ": Good. I got a NULL pointer which identifies end of k2 = " << k2 << ". i = " << i);
	}

      if (bs == 0) // EOF
	{
	  DMCS_LOG_TRACE(port << ": Got a NULL pointer");
	  break;
	}
      else
	{
	  DMCS_LOG_TRACE(port << ": Extract information from MQ");

	  std::size_t pa = ms.path;
	  std::size_t sid = ms.sid;
	  
	  DMCS_LOG_TRACE("My path = " << path << ", path from MQ pa = " << pa);
	  
	  assert (pa == path);

	  DMCS_LOG_TRACE(port << ": got #" << i);
	  DMCS_LOG_TRACE(port << ": bs = " << *bs << ", path = " << path << ", sid = " << sid << ". Notice: parent_session_id = " << parent_session_id);
	  ModelSessionId msi(bs, path, sid);
	  res->push_back(msi);
	}
    }

  DMCS_LOG_TRACE(port << ": DONE with the whole collect output");
}



void
OutputThread::write_result(connection_ptr conn,
			   ModelSessionIdListPtr& res)
{
  try
    {
      DMCS_LOG_TRACE(port << ": in write_result.");
      const std::string header = HEADER_ANS;
      conn->write(header);

      // now write the result
      //res->sort(my_compare);
      //res->unique();

      StreamingBackwardMessage return_mess(res);
  
      boost::asio::ip::tcp::socket& sock = conn->socket();
      boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
      DMCS_LOG_TRACE(port << ": return message ");
      DMCS_LOG_TRACE(port << ": " << return_mess << " to port " << ep.port() << ". Number of belief states = " << res->size());
      
      conn->write(return_mess);

      DMCS_LOG_TRACE(port << ": clean up");
      for (ModelSessionIdList::const_iterator it = res->begin(); it != res->end(); ++it)
	{
	  ModelSessionId ms = *it;
	  PartialBeliefState* result = ms.partial_belief_state;
	  delete result;
	  result = 0;
	}
    }
  catch (boost::system::error_code& e)
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
