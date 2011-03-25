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

OutputThread::OutputThread(std::size_t p, PathList pa)
  : port(p), 
    path(pa),
    output_buffer(new PartialBeliefStateBuf(CIRCULAR_BUF_SIZE))
{ }


OutputThread::~OutputThread()
{
  DMCS_LOG_TRACE(port << ": Terminating OutputThread.");
  for (PartialBeliefStateBuf::const_iterator it = output_buffer->begin(); 
       it != output_buffer->end(); ++it)
    {
      delete *it;
    }
  output_buffer->clear();
}



void
OutputThread::operator()(connection_ptr c,
			 MessagingGatewayBC* mg,
			 ConcurrentMessageQueue* hon)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  PathList path;
  std::size_t k1;
  std::size_t k2;
  std::size_t parent_session_id;
  ModelSessionIdListPtr res(new ModelSessionIdList);
  
  while (1)
    {
      res->clear();

      BaseNotification::NotificationType nt = wait_for_trigger(hon, path, k1, k2, parent_session_id);

      if (nt == BaseNotification::SHUTDOWN)
	{
	  return; // shutdown received
	}
      else 
	{
	  assert (nt == BaseNotification::REQUEST || nt == BaseNotification::NEXT);

	  collect_output(mg, res, k1, k2, parent_session_id);

	  DMCS_LOG_TRACE(port << ": Output collected. res->size() = " << res->size());

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
			       PathList& path,
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
OutputThread::send_empty_model(MessagingGatewayBC* mg)
{
#ifdef DEBUG
  History path(1, 0);
#else
  std::size_t path = 0;
#endif
  mg->sendModel(0, path, 0, 0, ConcurrentMessageQueueFactory::OUT_MQ, 0);
}



void
OutputThread::collect_output(MessagingGatewayBC* mg,
			     ModelSessionIdListPtr& res,
			     std::size_t k1,
			     std::size_t k2,
			     std::size_t parent_session_id)
{
  // collect up to pack_size models


  for (std::size_t i = 1; i <= k1; ++i)
    {
      DMCS_LOG_TRACE(port << ": Read from MQ. i = " << i);
      
      std::size_t prio       = 0;
      int timeout            = 200; // milisecs

      struct MessagingGatewayBC::ModelSession ms =
	mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

      DMCS_LOG_TRACE(port << ": Extract information from MQ");

      PartialBeliefState* bs = ms.m;
      std::size_t sid = ms.sid;
      
      if (bs == 0) // Ups, a NULL pointer
	{
	  //DMCS_LOG_TRACE(port << ": Got a NULL pointer. timeout == " << timeout);
	  if (timeout == 0)
	    {
	      DMCS_LOG_TRACE(port << ": TIME OUT");
	      --i;
	      continue;
	    }
	  else 
	    {
	      // timeout != 0 ==> either UNSAT or EOF
	      if (i == 1)
		{
		  DMCS_LOG_TRACE(port << ": TIME OUT at i == 1. RETURN NOW!");
		  return;
		}
	      else
		{
		  DMCS_LOG_TRACE(port << ": push back this NULL pointer to the message queue so that we will read it the next time");
		  send_empty_model(mg);
		  return;
		}
	    }
	}
      else
	{
	  DMCS_LOG_TRACE(port << ": got #" << i);
	  DMCS_LOG_TRACE(port << ": bs = " << *bs << ", sid = " << sid << ". Notice: parent_session_id = " << parent_session_id);
	  
	  bool was_cached;
	  store(bs, output_buffer, false, was_cached);

	  if (!was_cached)
	    {
	      DMCS_LOG_TRACE(port << ": Model was NOT cached, put it into result.");
	      // Got something in a reasonable time. Continue collecting.
	      ModelSessionId msi(bs, path, sid);
	      res->push_back(msi);
	    }
	  else
	    {
	      DMCS_LOG_TRACE(port << ": Model is cached, won't put it into result.");
	      --i;
	    }
	}
    } // for

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
      res->sort(my_compare);
      res->unique();

      StreamingBackwardMessage return_mess(res);
  
      boost::asio::ip::tcp::socket& sock = conn->socket();
      boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
      DMCS_LOG_TRACE(port << ": return message ");
      DMCS_LOG_TRACE(port << ": " << return_mess << " to port " << ep.port() << ". Number of belief states = " << res->size());
      
      conn->write(return_mess);
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
