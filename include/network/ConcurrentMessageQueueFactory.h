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

#include "dmcs/AskNextNotification.h"
#include "dmcs/StreamingForwardMessage.h"
#include "network/MessagingGateway.h"
#include "network/ConcurrentMessageQueue.h"

#include "mcs/BeliefState.h"
#include "solver/Conflict.h"

#include <boost/utility.hpp>
#include <boost/thread/once.hpp>
#include <boost/scoped_ptr.hpp>

namespace dmcs {

typedef MessagingGateway<PartialBeliefState, Decisionlevel, Conflict, StreamingForwardMessage, AskNextNotification> MessagingGatewayBC;
typedef boost::shared_ptr<MessagingGatewayBC> MessagingGatewayBCPtr;

const std::size_t DEFAULT_PACK_SIZE = 20;

class ConcurrentMessageQueueFactory : private boost::noncopyable
{
private:
  /// singleton instance
  static boost::scoped_ptr<ConcurrentMessageQueueFactory> ins;
  static boost::once_flag flag;
  
  /// global set of message queues
  std::map<std::size_t,std::vector<ConcurrentMessageQueuePtr> > mqs;
  /// mutex lock for mqs
  boost::mutex mtx;  

  /// create or get a ConcurrentMessageQueue
  ConcurrentMessageQueuePtr
  createMessageQueue(std::size_t uid, std::size_t no_nbs, std::size_t off, std::size_t max_k, std::size_t max_size);
  
  
  /// private ctor
  ConcurrentMessageQueueFactory();

  /// private copy ctor
  ConcurrentMessageQueueFactory(const ConcurrentMessageQueueFactory&);
  
public:
  
  /// MQ offsets
  enum MQIDs
    {
      OUT_MQ = 0,    // solver created belief state
      JOIN_OUT_MQ,   // joiner created belief state
      JOIN_IN_MQ,    // pairs of (neighbor_id, partial equilibria)
      REQUEST_MQ,    // requests from Handlers
      SAT_JOINER_MQ, // notification from SAT to Joiner, either to do the next join or to SHUT UP
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
   * @param k max queue size
   * 
   * @return a MessagingGateway
   */
  MessagingGatewayBCPtr
  createMessagingGateway(std::size_t uid, std::size_t no_nbs, std::size_t k);
  
  
  /** 
   * Creates a messaging gateway only with incoming and outgoing queues IN_MQ and OUT_MQ, resp.
   * 
   * @param uid a number that is unique to the whole computer
   * @param k max queue size
   * 
   * @return a MessagingGateway
   */
  MessagingGatewayBCPtr
  createMessagingGateway(std::size_t uid, std::size_t k);
  
};
  

} // namespace dmcs


#endif // _CONCURRENT_MESSAGE_QUEUE_FACTORY_H

// Local Variables:
// mode: C++
// End:

