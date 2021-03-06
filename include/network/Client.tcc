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
 * @file   Client.tcc
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:33:26 2009
 * 
 * @brief  
 * 
 * 
 */

#ifndef CLIENT_TCC
#define CLIENT_TCC

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "Client.h"
#include "Theory.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

namespace dmcs {

template<typename CmdType>
Client<CmdType>::Client(boost::asio::io_service& io_service,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			const std::string& my_header_,
			typename CmdType::input_type& mess_)
  : io_service_(io_service), conn(new connection(io_service)),
    my_header(my_header_), mess(mess_), 
    result(new (typename CmdType::value_type))
{
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

#if defined(DEBUG)
  std::cerr << "Client::Client()  " << endpoint << std::endl
	    << "Header to send  = " << my_header << std::endl
	    << "Message to send = " << mess << std::endl;
#endif //DEBUG

  conn->socket().async_connect(endpoint,
			       boost::bind(&Client::send_header, this,
					   boost::asio::placeholders::error,
					   ++endpoint_iterator));
}



template<typename CmdType>
void
Client<CmdType>::send_header(const boost::system::error_code& error,
			     boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
#ifdef DEBUG
  std::cerr << "Client::send_header" << std::endl;
#endif

  if (!error)
    {
      // first send the header so that the neighbor can recognize what
      // kind of message will be sent. Notice that the neighbor
      // receives just characters; hence, without the header, he won't
      // be able to choose the proper type for the message.

      conn->async_write(my_header, boost::bind(&Client::send_message, this,
					       boost::asio::placeholders::error, conn));

      /*
      std::vector<char> write_buf(my_header.begin(), my_header.end());
      std::cerr << "write_buf.size() = " << write_buf.size() << std::endl;
      
      boost::asio::async_write(conn->socket(), boost::asio::buffer(write_buf),
			       boost::bind(&Client::send_message, this,
			       boost::asio::placeholders::error, conn));*/
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      conn->socket().close();
      
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      conn->socket().async_connect(endpoint,
				   boost::bind(&Client::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator));
    }
  else
    {
      std::cerr << "Client::send_header: " << error.message() << std::endl;
    }
}



template<typename CmdType>
void
Client<CmdType>::send_message(const boost::system::error_code& error, connection_ptr conn)
{
#ifdef DEBUG
  std::cerr << "Client::send_message" << std::endl;
#endif // DEBUG

  if (!error)
    {
      std::cerr << mess << std::endl;
      conn->async_write(mess,
			boost::bind(&Client::read_header, this,
				    boost::asio::placeholders::error, conn));
    }
  else
    {
      std::cerr << "Client::send_message: " << error.message() << std::endl;
    }
}



template<typename CmdType>
void
Client<CmdType>::read_header(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Client::read_header" << std::endl;
#endif // DEBUG
      
      conn->async_read(received_header,
		       boost::bind(&Client::handle_read_header, this,
				   boost::asio::placeholders::error, conn));
    }
  else
    {
      std::cerr << "Client::read_header: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}


template<typename CmdType>
void
Client<CmdType>::handle_read_header(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#ifdef DEBUG
      std::cerr << "Client::handle_read_header" << std::endl
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
      std::cerr << "Client::handle_read_header: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}

template<typename CmdType>
void 
Client<CmdType>::read_answer(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Client::read_answer" << std::endl;
#endif //DEBUG

      conn->async_read(result,
		       boost::bind(&Client::read_header, this,
				   boost::asio::placeholders::error, conn));
    }
  else
    {
      std::cerr << "Client::read_answer: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}

template<typename CmdType>
void
Client<CmdType>::finalize(const boost::system::error_code& error, connection_ptr /* conn */)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Client::finalize" << std::endl;
#endif //DEBUG
      // Do nothing. Since we are not starting a new operation the
      // io_service will run out of work to do and the client will
      // exit.
    }
  else
    {
      std::cerr << "Client::finalize: " << error.message() << std::endl;
      throw std::runtime_error(error.message());
    }
}


} // namespace dmcs

#endif // CLIENT_TCC

// Local Variables:
// mode: C++
// End:
