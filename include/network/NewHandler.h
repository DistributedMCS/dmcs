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
 * @file   NewHandler.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Mar  9 15:13:14 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_HANDLER_H
#define NEW_HANDLER_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include "mcs/ForwardMessage.h"
#include "mcs/NewOutputDispatcher.h"
#include "network/connection.hpp"
#include "network/NewOutputThread.h"

namespace dmcs {

class NewHandler;
typedef boost::shared_ptr<NewHandler> NewHandlerPtr;

class NewHandler
{
public:
  NewHandler(std::size_t p);

  ~NewHandler();

  void
  startup(NewHandlerPtr handler,
	  connection_ptr conn,
	  NewConcurrentMessageDispatcherPtr md,
	  NewOutputDispatcherPtr od);
  
  void
  handle_read_header(const boost::system::error_code& e,
		     NewHandlerPtr handler,
		     connection_ptr conn,
		     NewConcurrentMessageDispatcherPtr md,
		     NewOutputDispatcherPtr od,
		     boost::shared_ptr<std::string> header);
  

  void
  handle_read_message(const boost::system::error_code& e,
		      NewHandlerPtr handler,
		      connection_ptr conn,
		      NewConcurrentMessageDispatcherPtr md,
		      NewOutputDispatcherPtr od,
		      ForwardMessage* mess);

  void
  handle_finalize(const boost::system::error_code& e,
		  NewHandlerPtr handler,
		  connection_ptr conn,
		  NewConcurrentMessageDispatcherPtr md,
		  ForwardMessage* mess);

private:
  bool first_round;
  std::size_t port;              // just for debugging information
  NewOutputThreadPtr output_sender;
  boost::thread* output_thread;
};
  
} // namespace dmcs

#endif // NEW_HANDLER_H

// Local Variables:
// mode: C++
// End:
