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

namespace dmcs {

class RequestDispatcher : public NewBaseDispatcher
{
public:
  RequestDispatcher(NewConcurrentMessageDispatcherPtr& md)
    : NewBaseDispatcher(md)
  { }

  void
  operator()()
  {
    while (1)
      {
	ForwardMessage* request = cmd->receive<ForwardMessage>(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, 0);
	std::size_t qid = request->query_id;

	if (shutdown(qid))
	  {
	    break;
	  }

	std::size_t ctx_id = ctxid_from_qid(qid);
	std::size_t offset = get_offset(ctx_id);

	cmd->send(NewConcurrentMessageDispatcher::REQUEST_MQ, offset, request, 0);
      }
  }
};

} // namespace dmcs

#endif // REQUEST_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
