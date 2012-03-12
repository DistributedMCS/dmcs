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
 * @file   NewServer.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Mar  11 20:20:20 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_SERVER_H
#define NEW_SERVER_H

#include "mcs/Registry.h"
#include "network/connection.hpp"
#include "network/NewHandler.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

class NewServer
{
public:
  NewServer(const RegistryPtr& r,
	    boost::asio::io_service& i,
	    const boost::asio::ip::tcp::endpoint& endpoint);

  ~NewServer();

  void
  handle_accept(const boost::system::error_code& e, 
		connection_ptr conn);

  void
  handle_read_header(const boost::system::error_code& e, 
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header);

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::size_t port;
  boost::mutex mtx;

  RegistryPtr reg;
  std::vector<NewHandler*> handler_vec;
  std::vector<boost::thread*> handler_thread_vec;
};

typedef boost::shared_ptr<NewServer> NewServerPtr;

} // namespace dmcs

#endif // NEW_SERVER_H

// Local Variables:
// mode: C++
// End:
