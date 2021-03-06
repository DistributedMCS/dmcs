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
 * @file   RequestDispatcher.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Jan  1 22:01:21 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef REQUEST_DISPATCHER_H
#define REQUEST_DISPATCHER_H

#include "mcs/NewBaseDispatcher.h"
#include "mcs/ForwardMessage.h"
#include "mcs/QueryID.h"

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

namespace dmcs {

class RequestDispatcher : public NewBaseDispatcher
{
public:
  RequestDispatcher()
  { }

  void
  startup(NewConcurrentMessageDispatcherPtr md)
  {    
    while (1)
      {
	int timeout = 0;
	ForwardMessage* request = md->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, timeout);

	if (request == NULL)
	  {
	    DBGLOG(DBG, "RequestDispatcher::startup(): got NULL. BREAK NOW!");
	    break;
	  }

	std::size_t qid = request->qid;
	std::size_t receiver = receiver_from_qid(qid);
	std::size_t offset = get_offset(receiver);
	const NewHistory& history = request->history;
	if (is_shutdown(qid))
	  {
	    DBGLOG(DBG, "RequestDispatcher::startup(): got shutdown qid. Forward to context and cycle breaker at offset = " << offset);
	    md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, offset, request, timeout);
	    md->send(NewConcurrentMessageDispatcher::CYCLE_BREAKER_MQ, offset, request, timeout);	    
	  }
	else
	  {
	    // not a shut_down request, let's continue
	    if (history.find(receiver) == history.end())
	      {
		md->send(NewConcurrentMessageDispatcher::REQUEST_MQ, offset, request, timeout);
	      }
	    else
	      {
		md->send(NewConcurrentMessageDispatcher::CYCLE_BREAKER_MQ, offset, request, timeout);
	      }
	  }
	boost::this_thread::interruption_point();
      }
  }
};

typedef boost::shared_ptr<RequestDispatcher> RequestDispatcherPtr;


struct RequestDispatcherWrapper
{
  void
  operator()(RequestDispatcherPtr rd, NewConcurrentMessageDispatcherPtr md)
  {
    rd->startup(md);
  }
};

} // namespace dmcs

#endif // REQUEST_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
