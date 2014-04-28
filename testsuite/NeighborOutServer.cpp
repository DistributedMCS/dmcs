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
 * @file   NeighborOutServer.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Mar  7 21:39:01 2012
 * 
 * @brief  
 * 
 * 
 */

#include "NeighborOutServer.h"
#include "mcs/QueryID.h"

using namespace dmcs;

NeighborOutServer::NeighborOutServer(boost::asio::io_service& i,
				     const boost::asio::ip::tcp::endpoint& endpoint)
  : io_service(i),
    acceptor(io_service, endpoint)
{ 
  std::cerr << "NEIGHBOR OUT SERVER: constructor()" << std::endl;

  m.qid = 0;
  m.k1 = 0;
  m.k2 = 0;

  connection_ptr my_connection(new connection(io_service));
  conn_man.insert(my_connection);
  
  acceptor.async_accept(my_connection->socket(),
			boost::bind(&NeighborOutServer::handle_accept, this,
				    boost::asio::placeholders::error, my_connection)
			);
}



void
NeighborOutServer::handle_accept(const boost::system::error_code& e, 
				connection_ptr conn)
{
  if (!acceptor.is_open())
    {
      return;
    }

  if (!e)
    {
      ForwardMessage* mess = new ForwardMessage(0, 0, 0);
      conn->async_read(*mess,
		       boost::bind(&NeighborOutServer::handle_read_message, this,
				   boost::asio::placeholders::error, conn, mess));
    }
  else
    {
      std::cerr << "NEIGHBOR OUT SERVER: ERROR handle_accept: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
NeighborOutServer::handle_read_message(const boost::system::error_code& e,
				       connection_ptr conn,
				       ForwardMessage* mess)
{
  if (!e)
    {
      if (is_shutdown(mess->qid))
	{
	  const std::string end_header = HEADER_TERMINATE;
	  conn->write(end_header);
	  
	  conn->socket().close();
	  acceptor.close();
	}
      else
	{
	  m.qid = mess->qid;
	  m.k1 = mess->k1;
	  m.k2 = mess->k2;

	  conn->async_read(*mess,
		       boost::bind(&NeighborOutServer::handle_read_message, this,
				   boost::asio::placeholders::error, conn, mess));
	}
    }
  else
    {
      std::cerr << "NEIGHBOR OUT SERVER: ERROR handle_read_message: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



ForwardMessage
NeighborOutServer::getMessage()
{
  return m;
}


// Local Variables:
// mode: C++
// End:
