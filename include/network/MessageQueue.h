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
  
  /// prefix string for the input MQs for partial equilibria from neighbors
#define DMCS_IN_MQ   "dmcs-in-mq"
  /// name of the SAT MQ, announces new models from the local solver
#define DMCS_SAT_MQ  "dmcs-sat-mq"
  /// name of the join MQ, announces that neighbor C_i sent k partial belief states
#define DMCS_JOIN_IN_MQ "dmcs-join-in-mq"
  /// name of the join MQ, announces joined belief states from the neighbors
#define DMCS_JOIN_OUT_MQ "dmcs-join-out-mq"
  /// name of the output MQ, announces partial equilibria
#define DMCS_OUT_MQ  "dmcs-out-mq"
  
  /// message size for all MQs except DMCS_JOIN_IN_MQ
#define DMCS_MQ_MSG_SIZE ( sizeof(void*) )

  /// message type for JoinIn messages
  struct JoinInMsg
  {
    std::size_t context_no;
    std::size_t peq_count;
  };

  /// message size for the DMCS_JOIN_IN_MQ message queue
#define DMCS_JOIN_IN_MSG_SIZE ( sizeof(JoinInMsg) )

  ///@todo TK: all MQs need to have a unique name on the system, thus
  ///we need to add the process ID of the creating process to the name
  ///and announce this PID to child processes/threads. on dmcs
  ///shutdown, we need to cleanup using MQ::remove(). note: all MQs
  ///will be created under /tmp/boost_interprocess and stay alive
  ///after program execution.

} // namespace dmcs


#endif // _MESSAGE_QUEUE_H

// Local Variables:
// mode: C++
// End:

