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
 * @file   MessageQueue.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sat Dec 18 14:02:46 2010
 * 
 * @brief  Basic message queues for IPC.
 * 
 * 
 */


#ifndef _MESSAGE_QUEUE_H
#define _MESSAGE_QUEUE_H

#include <boost/interprocess/ipc/message_queue.hpp>
#include <boost/shared_ptr.hpp>

namespace dmcs {

  /// a message queue
  typedef boost::interprocess::message_queue MQ;

  /// a message queue shared pointer
  typedef boost::shared_ptr<MQ> MQPtr;
  
} // namespace dmcs


#endif // _MESSAGE_QUEUE_H

// Local Variables:
// mode: C++
// End:

