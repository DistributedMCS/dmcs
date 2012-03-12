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
 * @file   NewServer.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Mar  11 20:28:30 2012
 * 
 * @brief  
 * 
 * 
 */

#include "network/NewServer.h"
#include <boost/thread/mutex.hpp>

namespace dmcs {

NewServer::NewServer(const RegistryPtr& r,
		     boost::asio::io_service& i,
		     const boost::asio::ip::tcp::endpoint& endpoint)
  : reg(r),
    io_service(i),
    acceptor(io_service, endpoint),
    port(endpoint.port())
{
  connection_ptr my_connection(new connection(io_service));

  acceptor.async_accept(my_connection->socket(),
			boost::bind(&NewServer::handle_accept, this,
				    boost::asio::placeholders::error,
				    my_connection)
			);
}



NewServer::~NewServer()
{
  for (std::vector<NewHandler*>::iterator it = handler_vec.begin();
       it != handler_vec.end(); ++it)
    {
      NewHandler* handler = *it;
      delete handler;
      handler = 0;
    }

  for (std::vector<boost::thread*>::iterator it = handler_thread_vec.begin();
       it != handler_thread_vec.end(); ++it)
    {
      boost::thread* handler_thread = *it;
      if (handler_thread->joinable())
	{
	  handler_thread->interrupt();
	  handler_thread->join();
	  delete handler_thread;
	  handler_thread = 0;
	}
    }
}



void
NewServer::handle_accept(const boost::system::error_code& e, 
			 connection_ptr conn)
{
  if (!e)
    {
      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor.io_service()));
      acceptor.async_accept(new_conn->socket(),
			    boost::bind(&NewServer::handle_accept, this,
					boost::asio::placeholders::error,
					new_conn)
			    );

      boost::shared_ptr<std::string> header(new std::string);
      
      conn->async_read(*header,
		       boost::bind(&NewServer::handle_read_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   header));
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}



void
NewServer::handle_read_header(const boost::system::error_code& e, 
			      connection_ptr conn,
			      boost::shared_ptr<std::string> header)
{
  boost::mutex::scoped_lock lock(mtx);

  if (!e)
    {
      if (header->find(HEADER_REQ_DMCS) != std::string::npos)
	{
	  boost::asio::ip::tcp::socket& sock = conn->socket();
	  boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 

	  NewHandler* handler = new NewHandler(ep.port());
	  boost::thread* handler_thread = new boost::thread(*handler,
							    conn,
							    reg->md, 
							    reg->od);

	  handler_vec.push_back(handler);
	  handler_thread_vec.push_back(handler_thread);
	}
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}



} // namespace dmcs

// Local Variables:
// mode: C++
// End:
