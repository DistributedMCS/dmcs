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
 * @file   ConcurrentMessageDispatcher.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Jan  9 08:41:50 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef _CONCURRENT_MESSAGE_DISPATCHER_H
#define _CONCURRENT_MESSAGE_DISPATCHER_H

#include "dmcs/BaseNotification.h"
#include "dmcs/StreamingForwardMessage.h"
#include "network/MessagingGateway.h"
#include "network/ConcurrentMessageQueue.h"
#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

#include <vector>
#include <string>


namespace dmcs {

  /**
   * @brief a dispatcher for concurrent message queues
   */
  class ConcurrentMessageDispatcher : public MessagingGateway<PartialBeliefState, 
							      Decisionlevel, 
							      Conflict, 
							      StreamingForwardMessage,
							      BaseNotification>
  {
  private:
    /// here we pile up all MQ's that receive messages
    std::vector<ConcurrentMessageQueuePtr> mqs;

    /// here we pile up the corresponding names of the MQs
    std::vector<std::size_t> mqids;

  public:
    /// default ctor
    ConcurrentMessageDispatcher();

    /// removes all MQs
    virtual
    ~ConcurrentMessageDispatcher();

    /** 
     * register a new MQ here
     *
     * @param mq the message queue
     * @param nm the name of the message queue
     */
    void
    registerMQ(ConcurrentMessageQueuePtr& mq, std::size_t id);

    virtual bool
    sendIncomingMessage(StreamingForwardMessage* sfMess, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0);

    virtual bool
    sendNotification(BaseNotification* notif, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0);

    virtual bool
    sendModel(PartialBeliefState* b, std::size_t path, std::size_t sid, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0);

#if 0
    virtual bool
    sendModelConflict(PartialBeliefState* b, Conflict* c, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0);

    virtual bool
    sendModelDecisionlevel(PartialBeliefState* b, Decisionlevel* d, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0);
#endif //0

    virtual bool
    sendJoinIn(std::size_t k, std::size_t from, std::size_t to, std::size_t prio, int msecs = 0);

    virtual StreamingForwardMessage*
    recvIncomingMessage(std::size_t from, std::size_t& prio, int& msecs);

    virtual BaseNotification*
    recvNotification(std::size_t from, std::size_t& prio, int& msecs);

    virtual struct ModelSession
    recvModel(std::size_t from, std::size_t& prio, int& msecs);

#if 0
    virtual struct ModelConflict
    recvModelConflict(std::size_t from, std::size_t& prio, int& msecs);

    virtual struct ModelDecisionlevel
    recvModelDecisionlevel(std::size_t from, std::size_t& prio, int& msecs);
#endif //0

    virtual struct JoinIn
    recvJoinIn(std::size_t from, std::size_t& prio, int& msecs);

  };

} // namespace dmcs


#endif // _CONCURRENT_MESSAGE_DISPATCHER_H

// Local Variables:
// mode: C++
// End:

