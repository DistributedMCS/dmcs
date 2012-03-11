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
  operator()(NewHandlerPtr hdl,
	     connection_ptr conn,
	     NewConcurrentMessageDispatcherPtr md,
	     NewOutputDispatcherPtr od,
	     ForwardMessage* first_mess);

  void
  handle_read_message(const boost::system::error_code& e,
		      NewHandlerPtr hdl,
		      connection_ptr conn,
		      NewConcurrentMessageDispatcherPtr md,
		      NewOutputDispatcherPtr od,
		      ForwardMessage* next_mess);

private:
  std::size_t port;              // just for debugging information
  NewOutputThread* output_sender;
  boost::thread* output_thread;
};
  
} // namespace dmcs

#endif // NEW_HANDLER_H

// Local Variables:
// mode: C++
// End:
