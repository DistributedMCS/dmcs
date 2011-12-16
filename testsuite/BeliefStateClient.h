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
 * @file   BeliefStateClient.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Dec  16 10:35:30 2011
 * 
 * @brief  
 * 
 * 
 */


#ifndef BELIEF_STATE_CLIENT_H
#define BELIEF_STATE_CLIENT_H

#include "network/connection.hpp"
#include "mcs/NewBeliefState.h"

class BeliefStateClient
{
public:
  BeliefStateClient(boost::asio::io_service& i,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		    dmcs::NewBeliefState* ws);

  void
  send_belief_state(const boost::system::error_code& e,
		    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		    connection_ptr conn);

  void
  finalize(const boost::system::error_code& e, connection_ptr conn);

  dmcs::NewBeliefState*
  bs_sent();

private:
  boost::asio::io_service& io_service;
  connection_ptr conn;
  dmcs::NewBeliefState* want_send;
};

#endif // BELIEF_STATE_CLIENT_H

// Local Variables:
// mode: C++
// End:
