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
							  const std::string& h,
							  ForwardMessType& fm)
  : BaseClient(io_service, endpoint_iterator, h),
    mess(fm)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

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

      if (received_header.compare(HEADER_EOF) == 0)
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
		       boost::bind(&AsynClient::handle_read_answer, this,
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
AsynClient<ForwardMessType, BackwardMessType>::handle_read_answer(const boost::system::error_code& e, connection_ptr conn)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  if (!e)
    {

      DMCS_LOG_TRACE("result = " << result);

      std::string header_next = HEADER_REQ_STM_DMCS;

      DMCS_LOG_TRACE("send " << header_next);

      //      std::cerr << "send " << header_next << std::endl;
      //      std::cerr << "result == " << result << std::endl;

      conn->async_write(header_next, boost::bind(&AsynClient::read_header, this,
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
