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
 * @file   OutputThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:25:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef OUTPUT_THREAD_H
#define OUTPUT_THREAD_H

#include "dmcs/OutputNotification.h"
#include "mcs/BeliefState.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/connection.hpp"
#include "solver/Conflict.h"

#include <boost/asio.hpp>

namespace dmcs {

class OutputThread
{
public:
  OutputThread();

  virtual
  ~OutputThread();

  void
  operator()(connection_ptr c,
	     std::size_t ps,
	     MessagingGatewayBC* m,
	     ConcurrentMessageQueue* hon);

private:
  bool
  wait_for_trigger(ConcurrentMessageQueue* handler_output_notif);

  void
  collect_output(MessagingGatewayBC* mg, PartialBeliefStateVecPtr& res, std::string& header);

  void
  write_result(connection_ptr conn,
	       PartialBeliefStateVecPtr& res,
	       const std::string& header);

  void
  handle_written_header(connection_ptr conn,
			PartialBeliefStateVecPtr& res);

private:
  std::size_t                pack_size;             // Number of models the invoker expects to get
  std::size_t                left_2_send;           // Number of models left to send
  bool                       collecting;           // A flag to determine whether we are in collecting mode or not 
                                                   // (if yes then we don't want to wait for any trigger)
};

typedef boost::shared_ptr<OutputThread> OutputThreadPtr;

} // namespace dmcs


#endif // OUTPUT_THREAD_H

// Local Variables:
// mode: C++
// End:
