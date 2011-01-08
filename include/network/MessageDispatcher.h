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
 * @file   MessageDispatcher.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Dec 21 05:59:31 2010
 * 
 * @brief Provides basic primitives to dispatch messages to multiple
 * message queues.
 * 
 * 
 */


#ifndef _MESSAGE_DISPATCHER_H
#define _MESSAGE_DISPATCHER_H

#include "network/MessagingGateway.h"
#include "network/MessageQueue.h"
#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

#include <vector>
#include <string>


namespace dmcs {

  /**
   * @brief a dispatcher for message queues
   */
  class MessageDispatcher : public MessagingGateway<BeliefState,Conflict>
  {
  private:
    /// here we pile up all MQ's that receive messages
    std::vector<MQPtr> mqs;

    /// here we pile up the corresponding names of the MQs
    std::vector<std::string> nms;

  public:
    /// default ctor
    MessageDispatcher();

    /// removes all MQs
    virtual
    ~MessageDispatcher();

    /** 
     * register a new MQ here
     *
     * @param mq the message queue
     * @param nm the name of the message queue
     */
    void
    registerMQ(MQPtr& mq, const std::string& nm);

    virtual void
    sendModel(BeliefState* b, std::size_t from, std::size_t to, std::size_t prio);

    virtual void
    sendConflict(Conflict* c, std::size_t from, std::size_t to, std::size_t prio);

    virtual void
    sendJoinIn(std::size_t k, std::size_t from, std::size_t to, std::size_t prio);

    virtual BeliefState*
    recvModel(std::size_t from, std::size_t& prio);

    virtual Conflict*
    recvConflict(std::size_t from, std::size_t& prio);

    virtual struct JoinIn
    recvJoinIn(std::size_t from, std::size_t& prio);

  };

} // namespace dmcs


#endif // _MESSAGE_DISPATCHER_H

// Local Variables:
// mode: C++
// End:

