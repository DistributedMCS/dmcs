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
 * @file   BeliefStateServer.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  16 10:26:30 2011
 * 
 * @brief  
 * 
 * 
 */

#include "BeliefStateServer.h"

#include <iostream>
#include <boost/thread.hpp>

BeliefStateServer::BeliefStateServer(boost::asio::io_service& i,
				     const boost::asio::ip::tcp::endpoint& endpoint)
  : io_service(i),
    acceptor(io_service, endpoint),
    bs(new dmcs::NewBeliefState(1))
{
  std::cerr << "SERVER: constructor()" << std::endl;

  connection_ptr my_connection(new connection(io_service));
  conn_man.insert(my_connection);
  
  acceptor.async_accept(my_connection->socket(),
			boost::bind(&BeliefStateServer::handle_accept, this,
				    boost::asio::placeholders::error, my_connection)
			);
}



void
BeliefStateServer::handle_accept(const boost::system::error_code& e, 
				 connection_ptr conn)
{
  if (!acceptor.is_open())
    {
      return;
    }

  if (!e)
    {
      std::cerr << "SERVER: Create new connection..." << std::endl;
      connection_ptr new_conn(new connection(acceptor.io_service()));
      conn_man.insert(new_conn);

      acceptor.async_accept(new_conn->socket(),
			    boost::bind(&BeliefStateServer::handle_accept, this,
					boost::asio::placeholders::error, new_conn)
			    );

      std::cerr << "SERVER: Wait for a message from BeliefStateClient" << std::endl;

      conn->async_read(*bs,
		       boost::bind(&BeliefStateServer::handle_finalize, this,
				   boost::asio::placeholders::error, conn, bs));
    }
  else
    {
      std::cerr << "SERVER: ERROR handle_accept: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
BeliefStateServer::handle_finalize(const boost::system::error_code& e, 
				   connection_ptr conn,
				   dmcs::NewBeliefState* bs)
{
  if (!e)
    {
      std::cerr << "SERVER: Got belief state = " << bs << ": " << *bs << std::endl;
      std::cerr << "SERVER: finalizing..." << std::endl;
      conn->socket().close();
      acceptor.close();
    }
  else
    {
      std::cerr << "SERVER: ERROR handle_read_header: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}


dmcs::NewBeliefState*
BeliefStateServer::bs_received()
{
  return bs;
}


// Local Variables:
// mode: C++
// End:
