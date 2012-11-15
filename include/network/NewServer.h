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
#include "network/NetworkPointers.h"
#include "network/connection.hpp"
#include "network/NewHandler.h"
#include "network/HandlerWrapper.h"
#include "network/NewThreadFactory.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

class NewServer
{
public:
  NewServer(boost::asio::io_service& i,
	    const boost::asio::ip::tcp::endpoint& endpoint,
	    const RegistryPtr r);

  ~NewServer();

  void
  handle_accept(const boost::system::error_code& e, 
		connection_ptr conn);

  void
  handle_read_header(const boost::system::error_code& e, 
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header);

  void
  send_notification_to_manager(const boost::system::error_code& e,
			       connection_ptr conn,
			       NewConcurrentMessageDispatcherPtr md,
			       boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

  void
  wait_trigger(const boost::system::error_code& e,
	       connection_ptr conn);

  void
  handle_read_trigger_message(const boost::system::error_code& e,
			      boost::shared_ptr<std::string> trigger_message,
			      connection_ptr conn);

  void
  connect_to_manager();

  void
  notify_shutdown_handler();

  bool
  isShutdown();

  void
  shutdown();

private:
  void
  first_initialization_phase();

  void
  second_initialization_phase();

private:
  boost::asio::io_service& io_service;
  boost::asio::ip::tcp::acceptor acceptor;
  std::size_t port;
  boost::mutex mtx;

  RegistryPtr reg;
  NewThreadFactoryPtr thread_factory;
  std::vector<HandlerWrapper*> handler_vec;
  BoostThreadVec handler_thread_vec;

  std::size_t count_shutdown_handlers;

  boost::shared_ptr<boost::asio::io_service> io_service_to_manager;
};

} // namespace dmcs

#endif // NEW_SERVER_H

// Local Variables:
// mode: C++
// End:
