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

template<typename Message_t>
Client<Message_t>::Client(boost::asio::io_service& io_service,
	       boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	       Message_t& mess_)
  : io_service_(io_service),
    conn(new connection(io_service)),
    mess(mess_),
    belief_states(new BeliefStateList)
{
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

#if defined(DEBUG)
  std::cerr << "Client::Client " << endpoint << std::endl;
#endif //DEBUG

  conn->socket().async_connect(endpoint,
			       boost::bind(&Client::handle_connect, this,
					   boost::asio::placeholders::error,
					   ++endpoint_iterator)
			       );
}



template<typename Message_t>
void
Client<Message_t>::handle_connect(const boost::system::error_code& error,
		       boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
#ifdef DEBUG
  std::cerr << "Handling connect" <<std::endl;
#endif

  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Client::handle_connect " << mess << std::endl;
#endif //DEBUG

      conn->async_write(mess,
			boost::bind(&Client::handle_write_message, this,
				    boost::asio::placeholders::error, conn));
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      conn->socket().close();
      
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      conn->socket().async_connect(endpoint,
				   boost::bind(&Client::handle_connect, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator));
    }
  else
    {
      // An error occurred. Log it and return. Since we are not starting a new
      // operation the io_service will run out of work to do and the client will
      // exit.
#ifdef DEBUG
      std::cerr << "handle_connect: " << error.message() << std::endl;
#endif
    }
}



template<typename Message_t>
void
Client<Message_t>::handle_write_message(const boost::system::error_code& error, 
			       connection_ptr /* c */)
{

#ifdef DEBUG
  std::cerr << "writing message" << std::endl;
#endif

  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Client::handle_write_message" << std::endl;
#endif // DEBUG
      
      //conn->async_read(*belief_states,
      conn->async_read(belief_states,
		       boost::bind(&Client::handle_read_models, this,
				   boost::asio::placeholders::error)
		       );
    }
  else
    {
      std::cerr << "handle_write_message: " << error.message() << std::endl;
    }
}


template<typename Message_t>
void 
Client<Message_t>::handle_read_models(const boost::system::error_code& error)
{

#ifdef DEBUG
  std::cerr << "reading models" << std::endl;
#endif

  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "At client: got some belief states..." << std::endl;
#endif //DEBUG
    }
  else
    {
      std::cerr << "handle_read_models: " << error.message() << std::endl;
    }

  // Since we are not starting a new operation the io_service will run out of
  // work to do and the client will exit.
}

} // namespace dmcs

#endif // CLIENT_TCC

// Local Variables:
// mode: C++
// End:
