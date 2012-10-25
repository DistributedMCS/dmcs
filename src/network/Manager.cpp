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
 * @file   Manager.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Oct  25 22:37:24 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/NewMessage.h"
#include "network/Manager.h"

namespace dmcs {

Manager::Manager(boost::asio::io_service& i,
		 const boost::asio::ip::tcp::endpoint& endpoint,
		 std::size_t system_size)
  : io_service(i),
    acceptor(io_service, endpoint),
    system_size(system_size)
{ 
  connection_ptr my_connection(new connection(io_service));

  DBGLOG(DBG, "Manager::ctor: waiting for new connection.");

  acceptor.async_accept(my_connection->socket(),
			boost::bind(&Manager::handle_accept, this,
				    boost::asio::placeholders::error,
				    my_connection)
			);
}



void
Manager::handle_accept(const boost::system::error_code& e, 
		       connection_ptr conn)
{ }



void
Manager::handle_read_header(const boost::system::error_code& e, 
			    connection_ptr conn,
			    boost::shared_ptr<std::string> header)
{ }


void
Manager::trigger_2nd_phase(const boost::system::error_code& e, 
			   connection_ptr conn)
{ }


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
