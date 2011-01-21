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
 * @file   AsynClient.tcc
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  10 14:22:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ASYN_CLIENT_TCC
#define ASYN_CLIENT_TCC

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "mcs/Theory.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "dmcs/StreamingForwardMessage.h"

#include "dmcs/Log.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

namespace dmcs {

template<typename ForwardMessType, typename BackwardMessType>
AsynClient<ForwardMessType, BackwardMessType>::AsynClient(boost::asio::io_service& io_service,
							  boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
							  const std::string& my_header_,
							  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_,
							  const NeighborPtr& nb_,
							  std::size_t ctx_id_,
							  std::size_t index_,
							  std::size_t pack_size_)
  : BaseClient(io_service, endpoint_iterator, my_header_),
    mess(ctx_id_, pack_size_),
    mg(mg_),
    nb(nb_), 
    ctx_id(ctx_id_),
    index(index_),
    pack_size(pack_size_)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

  DMCS_LOG_DEBUG("pack_size = " << pack_size);
  DMCS_LOG_DEBUG("endpoint  = " << endpoint);

  conn->socket().async_connect(endpoint,
			       boost::bind(&AsynClient::send_header, this,
					   boost::asio::placeholders::error,
					   ++endpoint_iterator));
}



template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::send_header(const boost::system::error_code& e,
							       boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      // The connection is now established successfully
      std::size_t prio = 0;
      std::size_t off = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + 2*index + 1;
      int timeout = 0;

      DMCS_LOG_DEBUG("offset = " << off);
      
      Conflict* conflict = mg->recvConflict(off, prio, timeout);
      mess.setConflict(conflict);
	
      DMCS_LOG_DEBUG("Send request to neighbor " << nb->neighbor_id << "@" << nb->hostname << ":" << nb->port);
	
      conn->async_write(my_header, boost::bind(&AsynClient::send_message, this,
					       boost::asio::placeholders::error, conn));
      
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      conn->socket().close();
      
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      conn->socket().async_connect(endpoint,
				   boost::bind(&AsynClient::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator));
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::send_message(const boost::system::error_code& e, connection_ptr conn)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("neighborMess = " << mess);

      conn->async_write(mess,
			boost::bind(&AsynClient::read_header, this,
				    boost::asio::placeholders::error, conn));
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::read_header(const boost::system::error_code& e, connection_ptr conn)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      conn->async_read(received_header,
		       boost::bind(&AsynClient::handle_read_header, this,
				   boost::asio::placeholders::error, conn));
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::handle_read_header(const boost::system::error_code& e, connection_ptr conn)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("Received header = " << received_header);

      if (received_header.compare("DMCS EOF") == 0)
	{
	  finalize(e, conn);
	}
      else
	{
	  read_answer(e, conn);
	}
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}

template<typename ForwardMessType, typename BackwardMessType>
void 
AsynClient<ForwardMessType, BackwardMessType>::read_answer(const boost::system::error_code& e, connection_ptr conn)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      conn->async_read(result,
		       boost::bind(&AsynClient::handle_answer, this,
		       boost::asio::placeholders::error, conn));
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
void 
AsynClient<ForwardMessType, BackwardMessType>::handle_answer(const boost::system::error_code& e, connection_ptr conn)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("result = " << result);

      // now put k models from result into a message queue. 
      const BeliefStateVecPtr bsv = result.getBeliefStates();
      const std::size_t off = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + 2*index;

      for (BeliefStateVec::const_iterator it = bsv->begin(); it != bsv->end(); ++it)
	{
	  BeliefState* bs = *it;
	  mg->sendModel(bs, ctx_id, off, 0);
	}

      // notify the joiner by putting a JoinMess into JoinMessageQueue
      mg->sendJoinIn(bsv->size(), ctx_id, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);


      // wait until the Joiner requests next models, then inform the
      // neighbor that I want next models
      std::string header_next = HEADER_NEXT;

      DMCS_LOG_DEBUG("send " << header_next);

      conn->async_write(header_next, boost::bind(&AsynClient::read_header, this,
						 boost::asio::placeholders::error, conn));

      read_header(boost::system::error_code(), conn);
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::finalize(const boost::system::error_code& e, connection_ptr /* conn */)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      // Do nothing. Since we are not starting a new operation the
      // io_service will run out of work to do and the client will
      // exit.
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


} // namespace dmcs

#endif // ASYN_CLIENT_TCC

// Local Variables:
// mode: C++
// End:
