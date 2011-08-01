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
 * @file   OutputDispatcher.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jul  28 17:18:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef OUTPUT_DISPATCHER_H
#define OUTPUT_DISPATCHER_H

#include "network/ConcurrentMessageQueueFactory.h"
#include "network/MessagingGateway.h"

namespace dmcs {

class OutputDispatcher
{
public:
  OutputDispatcher();

  void
  registerOutputThread(std::size_t, ConcurrentMessageQueue*);

  void
  operator()(MessagingGatewayBC* m);

private:
  CMQMapPtr output_thread_map;
};

}

#endif // OUTPUT_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
