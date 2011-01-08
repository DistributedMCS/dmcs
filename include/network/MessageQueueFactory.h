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

/// prefix string for the input MQ for conflicts from parents
#define DMCS_IN_MQ       "dmcs-in-mq-"
/// name of the conflict MQ, announces new conflicts from the local solver
#define DMCS_CONFLICT_MQ "dmcs-conflict-mq-"
/// name of the join MQ, announces that neighbor C_i sent k partial belief states
#define DMCS_JOIN_IN_MQ  "dmcs-join-in-mq-"
/// name of the join MQ, announces joined belief states from the neighbors
#define DMCS_JOIN_OUT_MQ "dmcs-join-out-mq-"
/// name of the output MQ, announces partial equilibria
#define DMCS_OUT_MQ      "dmcs-out-mq-"

#define INDEX_IN_MQ    0
#define INDEX_OUT_MQ   1
#define INDEX_CONFLICT 2
#define INDEX_JOIN_OUT 3
#define INDEX_JOIN_IN  4

  class MessageQueueFactory
  {
  private:
    MQPtr
    createMessageQueue(const char* name, std::size_t max_k, std::size_t max_size);

  public:
    enum MQIDs
      {
	IN_MQ = 0,
	OUT_MQ,
	CONFLICT_MQ,
	JOIN_OUT_MQ,
	JOIN_IN_MQ
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

