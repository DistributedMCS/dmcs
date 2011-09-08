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
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/OutputThread.h"

#include "dmcs/Log.h"

#include <boost/bind.hpp>
#include <boost/thread.hpp>



namespace dmcs {

OutputThread::OutputThread(std::size_t p)
  : port(p), 
    output_buffer(new PartialBeliefStateBuf(CIRCULAR_BUF_SIZE)),
    cmq(new ConcurrentMessageQueue())
{ }



OutputThread::~OutputThread()
{
  //DMCS_LOG_TRACE(port << ": Terminating OutputThread.");
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
OutputThread::operator()(connection_ptr c)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  std::size_t max_read = 0;
  std::size_t parent_session_id;
  ModelSessionIdListPtr res(new ModelSessionIdList);

  while (1)
    {
      res->clear();

      // read up to MAX_READ models or a NULL model
      std::size_t count_models = 0;
      while (count_models < MAX_READ)
	{
	  MessagingGatewayBC::ModelSession ms = receive_model(cmq.get());

	  // This is a stupid copy over! But let's keep it like this for now. 
	  // Unify the two types ModelSession and ModelSessionId later.
	  ModelSessionId msi(ms.m, ms.path, ms.sid);
	  res->push_back(msi);

	  PartialBeliefState* bs = ms.m;
	  if (bs == 0)
	    {
	      break;
	    }

	  count_models++;
	}
      
      assert (res->size() > 0);
      write_result(c, res);
    }
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



ConcurrentMessageQueue*
OutputThread::getCMQ()
{
  return cmq.get();
}



void
OutputThread::setPort(std::size_t p)
{
  port = p;
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
