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
 * @file   Handler.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Sep  29 7:26:32 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef HANDLER_H
#define HANDLER_H

#include "network/connection.hpp"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/Session.h"
#include "network/ThreadFactory.h"
#include "dmcs/CommandType.h"
#include "dmcs/StreamingCommandType.h"
#include "dyndmcs/InstantiatorCommandType.h"
#include "solver/Conflict.h"

#include "relsat-20070104/SATInstance.h"
#include "relsat-20070104/SATSolver.h"

#include <algorithm>
#include <cstdlib>
#include <list>
#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>


namespace dmcs {


template<typename CmdType>
class Handler
{
public:
  typedef Session<typename CmdType::input_type> SessionMsg;
  typedef typename boost::shared_ptr<SessionMsg> SessionMsgPtr;
  typedef typename boost::shared_ptr<CmdType> CmdTypePtr;

  //Handler(const CmdTypePtr cmd_, connection_ptr conn_);
  Handler(CmdTypePtr cmd, connection_ptr conn_);

  void 
  do_local_job(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd);

  void 
  send_result(typename CmdType::return_type result,
	      const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd);

  void 
  handle_session(const boost::system::error_code& e, SessionMsgPtr sesh, CmdTypePtr cmd);

  void 
  send_eof(const boost::system::error_code& e, SessionMsgPtr sesh);

  void 
  handle_finalize(const boost::system::error_code& e, SessionMsgPtr sesh);


private:
  connection_ptr conn;
};


// Specialized handler for streaming dmcs
template<>
class Handler<StreamingCommandType>
{
public:
  typedef Session<StreamingCommandType::input_type> SessionMsg;
  typedef boost::shared_ptr<SessionMsg> SessionMsgPtr;
  typedef boost::shared_ptr<StreamingCommandType> StreamingCommandTypePtr;

  //Handler(const CmdTypePtr cmd_, connection_ptr conn_);
  Handler(StreamingCommandTypePtr cmd, connection_ptr conn_);

  void 
  do_local_job(const boost::system::error_code& e, SessionMsgPtr sesh, StreamingCommandTypePtr cmd);

  void
  handle_read_header(const boost::system::error_code& e, SessionMsgPtr sesh, StreamingCommandTypePtr cmd);

private:
  connection_ptr conn;
  boost::thread* output_thread;
  std::string    header;
  std::size_t    port;
  bool           initialized;
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;
};

} // namespace dmcs


#include "network/Handler.tcc"

#endif // HANDLER_H

// Local Variables:
// mode: C++
// End:
