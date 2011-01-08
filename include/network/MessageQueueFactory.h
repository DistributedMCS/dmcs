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
 * @file   MessageQueueFactory.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Jan  4 11:22:45 2011
 * 
 * @brief  
 * 
 * 
 */


#ifndef _MESSAGE_QUEUE_FACTORY_H
#define _MESSAGE_QUEUE_FACTORY_H


#include "network/MessagingGateway.h"
#include "network/MessageQueue.h"

#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

namespace dmcs {

  class MessageQueueFactory
  {
  private:
    MQPtr
    createMessageQueue(const char* name, std::size_t max_k, std::size_t max_size);

  public:

    /// MQ offsets
    enum MQIDs
      {
	IN_MQ = 0, // parent context sent conflict
	OUT_MQ, // solver created belief state
	CONFLICT_MQ, // solver created conflict
	JOIN_OUT_MQ, // joiner created belief state
	JOIN_IN_MQ // neighbor i created k partial equilibria
      };


    MessageQueueFactory();


    virtual
    ~MessageQueueFactory();


    /** 
     * Creates a messaging gateway with the whole set of queues.
     * 
     * @param uid a number that is unique to the whole computer
     * @param no_nbs no_nbs is the number of neighbors
     * 
     * @return a MessagingGateway
     */
    boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >
    createMessagingGateway(std::size_t uid, std::size_t no_nbs);


    /** 
     * Creates a messaging gateway only with incoming and outgoing queues IN_MQ and OUT_MQ, resp.
     * 
     * @param uid a number that is unique to the whole computer
     * 
     * @return a MessagingGateway
     */
    boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >
    createMessagingGateway(std::size_t uid);

  };


} // namespace dmcs


#endif // _MESSAGE_QUEUE_FACTORY_H

// Local Variables:
// mode: C++
// End:

