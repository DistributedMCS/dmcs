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
 * @file   BeliefStateServer.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  16 10:12:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BELIEF_STATE_SERVER_H
#define BELIEF_STATE_SERVER_H

#include "network/connection.hpp"
#include "mcs/NewBeliefState.h"

class BeliefStateServer
{
public:
  BeliefStateServer(boost::asio::io_service& i,
		    const boost::asio::ip::tcp::endpoint& endpoint);

  void
  handle_accept(const boost::system::error_code& e, 
		connection_ptr conn);

  void
  handle_finalize(const boost::system::error_code& e, 
		  connection_ptr conn,
		  dmcs::NewBeliefState* bs);

  dmcs::NewBeliefState*
  bs_received();

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::set<connection_ptr> conn_man;
  dmcs::NewBeliefState* bs;
};

#endif // BELIEF_STATE_SERVER_H

// Local Variables:
// mode: C++
// End:
