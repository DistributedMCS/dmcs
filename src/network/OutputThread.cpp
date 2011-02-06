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

OutputThread::OutputThread(std::size_t i, std::size_t p)
  : invoker(i), port(p)
{ }


OutputThread::~OutputThread()
{
  DMCS_LOG_TRACE(port << ": So long, suckers. I'm out.");
}



void
OutputThread::operator()(connection_ptr c,
			 bool return_all,
			 MessagingGatewayBC* mg,
			 ConcurrentMessageQueue* hon)
{
  if (return_all)
    {
      // only output in an unlimited manner at the root context is
      // allowed. However, each time we output not more than
      // DEFAULT_PACKAGE_SIZE models

      assert (invoker == 0);
      output_all(c, mg);
    }
  else
    {
      output_limit(c, mg, hon);
    }
}



void
OutputThread::output_all(connection_ptr conn, MessagingGatewayBC* mg)
{
  while (1)
    {
      PartialBeliefStateVecPtr res(new PartialBeliefStateVec);
      VecSizeTPtr res_sid(new VecSizeT);
      std::string header;

      left_2_send = DEFAULT_PACK_SIZE;
      collect_output(mg, res, res_sid, header);
      write_result(conn, res, res_sid, header);
    }
}



void
OutputThread::output_limit(connection_ptr c,
			   MessagingGatewayBC* m,
			   ConcurrentMessageQueue* hon)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  collecting = false;
  eof_left = false;

  while (1)
    {
      PartialBeliefStateVecPtr res(new PartialBeliefStateVec);
      VecSizeTPtr res_sid(new VecSizeT);
      std::string header;

      if (eof_left)
	{
	  assert (!collecting);
	}

      if (!collecting)
	{
	  if (!wait_for_trigger(hon))
	    {
	      return; // shutdown received
	    }
	}

      if (eof_left)
	{
	  DMCS_LOG_TRACE(port << ": Send leftover EOF");
	  const std::string header_eof = HEADER_EOF;
	  c->write(header_eof);
	  eof_left = false;
	}
      else
	{
	  DMCS_LOG_TRACE(port << ": Go to collect output");
	  if (collect_output(m, res, res_sid, header))
	    {
	      write_result(c, res, res_sid, header);
	    }
	}
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

  DMCS_LOG_TRACE(port << ": Wait for message from Handler");

  handler_output_notif->receive(ptr, sizeof(on), recvd, p);

  if (ptr && on)
    {
      if (on->type == OutputNotification::REQUEST)
	{
	  pack_size = on->pack_size;
	  left_2_send = on->pack_size;
	  parent_session_id = on->parent_session_id;
	  retval = true;

	  DMCS_LOG_TRACE(port << ": Got a message from Handler. pack_size = " << pack_size << ". parent_session_id = " << parent_session_id);
	}
      else if (on->type == OutputNotification::SHUTDOWN)
	{
	  retval = false;

	  DMCS_LOG_TRACE(port << ": Got SHUTDOWN from Handler.");
	}
      else
	{
	  assert (false);
	}
    }
  else
    {
      DMCS_LOG_FATAL("Got null message: " << ptr << " " << on);
      assert(ptr != 0 && on != 0);
    }

  return retval;
}



// return true if we actually collected something
bool
OutputThread::collect_output(MessagingGatewayBC* mg,
			     PartialBeliefStateVecPtr& res,
			     VecSizeTPtr& res_sid,
			     std::string& header)
{
  DMCS_LOG_TRACE(port << ": pack_size = " << pack_size << ", left to send = " << left_2_send);

  // Turn this mode on. It's off only when either we collect and send
  // enough pack_size models, or there's no more model to send.
  collecting = true; 
  header = HEADER_ANS;
  
  for (std::size_t i = 1; i <= left_2_send; ++i)
    {
      //DMCS_LOG_TRACE(port << ":  Read from MQ");
      
      std::size_t prio       = 0;
      int timeout            = 200; // milisecs

      struct MessagingGatewayBC::ModelSession ms =
	mg->recvModel(ConcurrentMessageQueueFactory::OUT_MQ, prio, timeout);

      PartialBeliefState* bs = ms.m;
      std::size_t sid = ms.sid; ///@todo FIXME
      
      //DMCS_LOG_TRACE(port << ":  Check result from MQ");
      if (bs == 0) // Ups, a NULL pointer
	{
	  //DMCS_LOG_TRACE(port << ": Got a NULL pointer. timeout == " << timeout);
	  if (timeout == 0)
	    {
	      //DMCS_LOG_TRACE(port << ":  TIME OUT. Going to send whatever I got so far to the parent");
	      if (res->size() > 0)
		{
		  DMCS_LOG_TRACE(port << ": Going to send HEADER_ANS");
		  header = HEADER_ANS;
		  break;
		}
	      else
		{
		  //DMCS_LOG_TRACE(port << ": Timeout after 200msecs, continuing with " << i-1 << "th bs.");

		  // decrease counter because we gained nothing so far.
		  --i;
		  continue;
		}
	    }
	  else 
	    {
	      // timeout != 0 ==> either UNSAT or EOF
	      if (sid == parent_session_id)
		{
		  DMCS_LOG_TRACE(port << ": Got UNSAT or EOF. sid = " << sid << ", parent_session_id = " << parent_session_id << ". Turn off collecting mode.");
		  header = HEADER_EOF;
		  collecting = false;
		  break;
		}
	      else
		{
		  DMCS_LOG_TRACE(port << ": Got UNSAT or EOF. sid = " << sid << ", parent_session_id = " << parent_session_id << ". IGNORE.");
		  --i;
		  continue;
		}
	    }
	}
      
      DMCS_LOG_TRACE(port << ":  got #" << i << ": bs = " << *bs << ", sid = " << sid << ". Notice: parent_session_id = " << parent_session_id);
      
      // Got something in a reasonable time. Continue collecting.

      // It is weird if sid > parent_session_id, because SAT solver
      // always sends a NULL pointer when it is interrupted
      assert (sid < parent_session_id || sid == parent_session_id);

      if (sid == parent_session_id)
	{
	  res->push_back(bs);
	  res_sid->push_back(sid);
	}
    } // for

  if (res->size() > 0)
    {
      return true;
    }

  return false;
}


void
OutputThread::write_result(connection_ptr conn,
			   PartialBeliefStateVecPtr& res,
			   VecSizeTPtr& res_sid,
			   const std::string& header)
{
  DMCS_LOG_TRACE(port << ": header = " << header);

  try
    {
      eof_left = false;
      if (header.find(HEADER_ANS) != std::string::npos)
	{
	  // send some models
	  conn->write(header);
	  handle_written_header(conn, res, res_sid);
	}
      else
	{
	  assert (header.find(HEADER_EOF) != std::string::npos);
	  
	  if (res->size() > 0)
	    {
	      // send lefover models
	      DMCS_LOG_TRACE(port << ": Going to send leftover answers and keep EOF for the next round");
	      eof_left = true;
	      const std::string& ans_header = HEADER_ANS;
	      conn->write(ans_header);
	      handle_written_header(conn, res, res_sid);

	      if (invoker == 0)
		{
		  DMCS_LOG_TRACE(port << ": The BOSS called, notify him about EOF");
		  conn->write(header);
		  eof_left = false;
		}
	    }
	  else
	    {
	      DMCS_LOG_TRACE(port << ": NOTHING TO OUTPUT, going to send EOF");
	      conn->write(header);
	    }
	}
    }
  catch (boost::system::error_code& e)
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }

  //  assert(false && "header is neither HEADER_ANS nor HEADER_EOF");
}



void
OutputThread::handle_written_header(connection_ptr conn,
				    PartialBeliefStateVecPtr& res,
				    VecSizeTPtr& res_sid)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  assert (left_2_send >= res->size());
  
  left_2_send -= res->size();
      
  // mark end of package by a NULL model
  if (left_2_send == 0 || eof_left)
    {
      PartialBeliefState* bs = 0;
      res->push_back(bs);
      res_sid->push_back(0);
      collecting = false;
    }
      
  StreamingBackwardMessage return_mess(res, res_sid);
  
  boost::asio::ip::tcp::socket& sock = conn->socket();
  boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
      
  DMCS_LOG_TRACE(port << ": return message " << return_mess << " to port " << ep.port());
      
  conn->write(return_mess);
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
