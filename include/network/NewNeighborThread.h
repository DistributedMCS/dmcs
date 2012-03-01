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
 * @file   NewNeighborThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Mar  1 14:34:24 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_NEIGHBOR_THREAD_H
#define NEW_NEIGHBOR_THREAD_H

#include "mcs/BackwardMessage.h"
#include "mcs/NewNeighbor.h"
#include "network/NewConcurrentMessageDispatcher.h"
#include "network/connection.hpp"

#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace dmcs {

class NewNeighborThread
{
public:
  NewNeighborThread(NewNeighborPtr n);

  ~NewNeighborThread();

  void
  operator()(NewConcurrentMessageDispatcherPtr md);

private:
  void
  establish_connections(const boost::system::error_code& e,
			connection_ptr conn,
			NewConcurrentMessageDispatcherPtr md,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

  void
  handle_read_header(const boost::system::error_code& e,
		     connection_ptr conn,
		     NewConcurrentMessageDispatcherPtr md,
		     boost::shared_ptr<std::string> header);

  void
  handle_read_message(const boost::system::error_code& e,
		      connection_ptr conn,
		      NewConcurrentMessageDispatcherPtr md,
		      BackwardMessagePtr mess);

private:
  NewNeighborPtr neighbor;
  boost::thread* nop_thread;    // the corresponding neighbor output thread
  boost::shared_ptr<boost::asio::io_service> io_service;
};

} // namespace dmcs

#endif // NEW_NEIGHBOR_THREAD_H

// Local Variables:
// mode: C++
// End:
