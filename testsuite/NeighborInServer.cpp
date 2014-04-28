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
 * @file   NeighborInServer.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Mar  7 10:19:02 2012
 * 
 * @brief  
 * 
 * 
 */

#include "NeighborInServer.h"
#include "mcs/BackwardMessage.h"

using namespace dmcs;

NeighborInServer::NeighborInServer(boost::asio::io_service& i,
				   const boost::asio::ip::tcp::endpoint& endpoint,
				   ReturnedBeliefStateListPtr ol)
  : io_service(i),
    acceptor(io_service, endpoint),
    output_list(ol)
{ 
  std::cerr << "NEIGHBOR IN SERVER: constructor()" << std::endl;

  connection_ptr my_connection(new connection(io_service));
  conn_man.insert(my_connection);
  
  acceptor.async_accept(my_connection->socket(),
			boost::bind(&NeighborInServer::handle_accept, this,
				    boost::asio::placeholders::error, my_connection)
			);
}



void
NeighborInServer::handle_accept(const boost::system::error_code& e, 
				connection_ptr conn)
{
  if (!acceptor.is_open())
    {
      return;
    }

  if (!e)
    {
      std::cerr << "NEIGHBOR IN SERVER: Write result" << std::endl;
      
      const std::string header = HEADER_ANS;
      conn->write(header);
      
      BackwardMessage bmess(output_list);
      
      conn->write(bmess);
      
      const std::string end_header = HEADER_TERMINATE;
      conn->write(end_header);

      std::cerr << "NEIGHBOR IN SERVER: Finalizing..." << std::endl;
      conn->socket().close();
      acceptor.close();
    }
  else
    {
      std::cerr << "NEIGHBOR IN SERVER: ERROR handle_accept: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}

// Local Variables:
// mode: C++
// End:
