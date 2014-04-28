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
 * @file   NeighborOutServer.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Mar  7 21:37:40 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEIGHBOR_OUT_SERVER_TEST_H
#define NEIGHBOR_OUT_SERVER_TEST_H

#include "network/connection.hpp"
#include "mcs/ForwardMessage.h"

class NeighborOutServer
{
public:
  NeighborOutServer(boost::asio::io_service& i,
		    const boost::asio::ip::tcp::endpoint& endpoint);

  void
  handle_accept(const boost::system::error_code& e, 
		connection_ptr conn);

  void
  handle_read_message(const boost::system::error_code& e, 
		      connection_ptr conn,
		      dmcs::ForwardMessage* mess);

  dmcs::ForwardMessage
  getMessage();

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::set<connection_ptr> conn_man;

  dmcs::ForwardMessage m;
};

#endif // NEIGHBOR_OUT_SERVER_TEST_H

// Local Variables:
// mode: C++
// End:
