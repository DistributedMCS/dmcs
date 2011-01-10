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

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

namespace dmcs {

template<typename InputType>
AsynClient<InputType>::AsynClient(boost::asio::io_service& io_service,
			  boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			  const std::string& my_header_,
			  InputType& mess_)
  : BaseClient(io_service, endpoint_iterator, my_header_),
    mess(mess_)
{
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

#if defined(DEBUG)
  std::cerr << "AsynClient::AsynClient()  " << endpoint << std::endl
	    << "Header to send  = " << my_header << std::endl
	    << "Message to send = " << mess << std::endl;
#endif //DEBUG

  conn->socket().async_connect(endpoint,
			       boost::bind(&AsynClient::send_header, this,
					   boost::asio::placeholders::error,
					   ++endpoint_iterator));
}



template<typename InputType>
void
AsynClient<InputType>::send_header(const boost::system::error_code& error,
			     boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
#ifdef DEBUG
  std::cerr << "AsynClient::send_header" << std::endl;
#endif

  if (!error)
    {
      // first send the header so that the neighbor can recognize what
      // kind of message will be sent. Notice that the neighbor
      // receives just characters; hence, without the header, he won't
      // be able to choose the proper type for the message.

      conn->async_write(my_header, boost::bind(&AsynClient::send_message, this,
					       boost::asio::placeholders::error, conn));

      /*
      std::vector<char> write_buf(my_header.begin(), my_header.end());
      std::cerr << "write_buf.size() = " << write_buf.size() << std::endl;
      
      boost::asio::async_write(conn->socket(), boost::asio::buffer(write_buf),
			       boost::bind(&AsynClient::send_message, this,
			       boost::asio::placeholders::error, conn));*/
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
      std::cerr << "AsynClient::send_header: " << error.message() << std::endl;
    }
}



template<typename InputType>
void
AsynClient<InputType>::send_message(const boost::system::error_code& error, connection_ptr conn)
{
#ifdef DEBUG
  std::cerr << "AsynClient::send_message" << std::endl;
#endif // DEBUG

  if (!error)
    {
      std::cerr << mess << std::endl;
      conn->async_write(mess,
			boost::bind(&AsynClient::read_header, this,
				    boost::asio::placeholders::error, conn));
    }
  else
    {
      std::cerr << "AsynClient::send_message: " << error.message() << std::endl;
    }
}



template<typename InputType>
void
AsynClient<InputType>::read_header(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "AsynClient::read_header" << std::endl;
#endif // DEBUG
      
      conn->async_read(received_header,
		       boost::bind(&AsynClient::handle_read_header, this,
				   boost::asio::placeholders::error, conn));
    }
  else
    {
      std::cerr << "AsynClient::read_header: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}


template<typename InputType>
void
AsynClient<InputType>::handle_read_header(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#ifdef DEBUG
      std::cerr << "AsynClient::handle_read_header" << std::endl
		<< "Received header = " << received_header << std::endl;
#endif

      if (received_header.compare("DMCS EOF") == 0)
	{
	  finalize(error, conn);
	}
      else
	{
	  read_answer(error, conn);
	}
    }
  else
    {
      std::cerr << "AsynClient::handle_read_header: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}

template<typename InputType>
void 
AsynClient<InputType>::read_answer(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "AsynClient::read_answer" << std::endl;
#endif //DEBUG

      /*      conn->async_read(result,
		       boost::bind(&AsynClient::read_header, this,
		       boost::asio::placeholders::error, conn));*/
    }
  else
    {
      std::cerr << "AsynClient::read_answer: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}

template<typename InputType>
void
AsynClient<InputType>::finalize(const boost::system::error_code& error, connection_ptr /* conn */)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "AsynClient::finalize" << std::endl;
#endif //DEBUG
      // Do nothing. Since we are not starting a new operation the
      // io_service will run out of work to do and the client will
      // exit.
    }
  else
    {
      std::cerr << "AsynClient::finalize: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}


} // namespace dmcs

#endif // ASYN_CLIENT_TCC

// Local Variables:
// mode: C++
// End:
