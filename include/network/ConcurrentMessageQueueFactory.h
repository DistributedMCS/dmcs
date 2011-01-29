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
 * @file   ConcurrentMessageQueueFactory.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Jan  9 09:49:41 2011
 * 
 * @brief  
 * 
 * 
 */


#ifndef _CONCURRENT_MESSAGE_QUEUE_FACTORY_H
#define _CONCURRENT_MESSAGE_QUEUE_FACTORY_H


#include "network/MessagingGateway.h"
#include "network/ConcurrentMessageQueue.h"

#include "mcs/BeliefState.h"
#include "solver/Conflict.h"


#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>

namespace dmcs {

typedef MessagingGateway<PartialBeliefState, Decisionlevel, Conflict> MessagingGatewayBC;
typedef boost::shared_ptr<MessagingGatewayBC> MessagingGatewayBCPtr;

class ConcurrentMessageQueueFactory : private boost::noncopyable
{
private:
  /// singleton instance
  static boost::scoped_ptr<ConcurrentMessageQueueFactory> ins;
  static boost::once_flag flag;
  
  /// global set of message queues
  std::map<std::size_t,ConcurrentMessageQueuePtr> mqs;
  
  /// create or get a ConcurrentMessageQueue
  ConcurrentMessageQueuePtr
  createMessageQueue(std::size_t id, std::size_t max_k, std::size_t max_size);
  
  
  /// private ctor
  ConcurrentMessageQueueFactory();

  /// private copy ctor
  ConcurrentMessageQueueFactory(const ConcurrentMessageQueueFactory&);
  
public:
  
  /// MQ offsets
  enum MQIDs
    {
      IN_MQ = 0,   // parent context sent conflict
      OUT_MQ,      // solver created belief state
      CONFLICT_MQ, // solver created conflict
      JOIN_OUT_MQ, // joiner created belief state
      JOIN_IN_MQ,  // pairs of (neighbor_id, partial equilibria)
      NEIGHBOR_MQ
    };
  
  
  static ConcurrentMessageQueueFactory&
  instance();
  
  
  static void
  init(); // never throws
  
  
  virtual
  ~ConcurrentMessageQueueFactory();

  
  /** 
   * Creates a messaging gateway with the whole set of queues.
   * 
   * @param uid a number that is unique to the whole computer
   * @param no_nbs no_nbs is the number of neighbors
   * 
   * @return a MessagingGateway
   */
  MessagingGatewayBCPtr
  createMessagingGateway(std::size_t uid, std::size_t no_nbs);
  
  
  /** 
   * Creates a messaging gateway only with incoming and outgoing queues IN_MQ and OUT_MQ, resp.
   * 
   * @param uid a number that is unique to the whole computer
   * 
   * @return a MessagingGateway
   */
  MessagingGatewayBCPtr
  createMessagingGateway(std::size_t uid);
  
};
  

} // namespace dmcs


#endif // _CONCURRENT_MESSAGE_QUEUE_FACTORY_H

// Local Variables:
// mode: C++
// End:

