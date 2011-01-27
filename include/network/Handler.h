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

#include "network/BaseHandler.h"
#include "network/connection.hpp"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/Session.h"
#include "network/OutputThread.h"
#include "network/ThreadFactory.h"
#include "network/Server.h"
#include "network/StreamingDMCSThread.h"
#include "dmcs/CommandType.h"
#include "dmcs/StreamingCommandType.h"
#include "dyndmcs/InstantiatorCommandType.h"
#include "solver/Conflict.h"

#include "relsat-20070104/SATInstance.h"
#include "relsat-20070104/SATSolver.h"

#include <algorithm>
#include <cstdlib>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>


namespace dmcs {



template<typename CmdType>
class Handler : public BaseHandler<CmdType>
{
public:
  Handler();

  void
  do_local_job(const boost::system::error_code& e,
	       typename BaseHandler<CmdType>::HandlerPtr hdl,
	       typename BaseHandler<CmdType>::SessionMsgPtr sesh,
	       typename BaseHandler<CmdType>::CmdTypePtr cmd,
	       bool first_call);

  void 
  send_result(const boost::system::error_code& e,
	      typename CmdType::return_type result,
	      typename BaseHandler<CmdType>::HandlerPtr hdl,
	      typename BaseHandler<CmdType>::SessionMsgPtr sesh,
	      typename BaseHandler<CmdType>::CmdTypePtr cmd);

  void 
  handle_session(const boost::system::error_code& e,
		 typename BaseHandler<CmdType>::HandlerPtr hdl,
		 typename BaseHandler<CmdType>::SessionMsgPtr sesh,
		 typename BaseHandler<CmdType>::CmdTypePtr cmd);

  void 
  send_eof(const boost::system::error_code& e,
	   typename BaseHandler<CmdType>::HandlerPtr hdl,
	   typename BaseHandler<CmdType>::SessionMsgPtr sesh);

  void 
  handle_finalize(const boost::system::error_code& e,
		  typename BaseHandler<CmdType>::HandlerPtr hdl,
		  typename BaseHandler<CmdType>::SessionMsgPtr sesh);

};



// **********************************************************************************************************************
// Specialized handler for streaming dmcs
template<>
class Handler<StreamingCommandType> : public BaseHandler<StreamingCommandType>
{
public:
  Handler();

  virtual
  ~Handler();

  void 
  do_local_job(const boost::system::error_code& e,
	       BaseHandler<StreamingCommandType>::HandlerPtr hdl,
	       BaseHandler<StreamingCommandType>::SessionMsgPtr sesh,
	       BaseHandler<StreamingCommandType>::CmdTypePtr cmd,
	       bool first_call);


  void
  handle_read_header(const boost::system::error_code& e,
		     BaseHandler<StreamingCommandType>::HandlerPtr hdl,
		     BaseHandler<StreamingCommandType>::SessionMsgPtr sesh,
		     BaseHandler<StreamingCommandType>::CmdTypePtr cmd,
		     boost::shared_ptr<std::string> header);


private:
  boost::thread*                   output_thread;
  boost::thread*                   streaming_dmcs_thread;

  ConcurrentMessageQueuePtr        handler_dmcs_notif;
  ConcurrentMessageQueuePtr        handler_output_notif;

  MessagingGatewayBCPtr            mg;
};

} // namespace dmcs


#include "network/Handler.tcc"

#endif // HANDLER_H

// Local Variables:
// mode: C++
// End:
