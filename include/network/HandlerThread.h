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
 * @file   HandlerThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Mar  23 10:19:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef HANDLER_THREAD_H
#define HANDLER_THREAD_H

//#include "network/Handler.h"
#include "network/ConcurrentMessageQueueFactory.h"

#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcs {

  class StreamingCommandType;
  class StreamingForwardMessage;

  template<typename T>
  class Handler;

  template<typename T>
  class Session;

typedef Handler<StreamingCommandType> StreamingHandler;
typedef boost::shared_ptr<StreamingHandler > StreamingHandlerPtr;
typedef Session<StreamingForwardMessage > StreamingSessionMsg;
typedef boost::shared_ptr<StreamingSessionMsg > StreamingSessionMsgPtr;

class HandlerThread
{
public:
  HandlerThread(std::size_t i)
    : invoker(i)
  { }

  void
  operator()(StreamingHandlerPtr handler,
	     StreamingSessionMsgPtr sesh,
	     MessagingGatewayBC* mg);

private:
  std::size_t invoker;
};



typedef std::vector<HandlerThread*> HandlerThreadVec;
typedef boost::shared_ptr<HandlerThreadVec> HandlerThreadVecPtr;



} // namespace dmcs


#endif // STREAMING_DMCS_THREAD_H

// Local Variables:
// mode: C++
// End:
