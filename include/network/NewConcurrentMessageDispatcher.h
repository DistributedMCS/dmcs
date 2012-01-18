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
 * @file   NewConcurrentMessageDispatcher.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  28 17:28:51 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef NEW_CONCURRENT_MESSAGE_DISPATCHER_H
#define NEW_CONCURRENT_MESSAGE_DISPATCHER_H

#include <vector>
#include <boost/shared_ptr.hpp>

#include "network/ConcurrentMessageQueue.h"

namespace dmcs {

/**
 * @brief a dispatcher for concurrent message queues
 */
class NewConcurrentMessageDispatcher
{
public:
  enum MQIDs
    {
      REQUEST_DISPATCHER_MQ = 0, // 1-dimensional queues
      OUTPUT_DISPATCHER_MQ,      // all stored in cmqs[0]
      JOINER_DISPATCHER_MQ,
      SEPARATOR,                 // separating 1-dimensional to 2-dimensional queues
      REQUEST_MQ,                // use: id - SEPARATOR to get the index of the vector
      NEIGHBOR_OUT_MQ,           //      storing all queues of type id
      NEIGHBOR_IN_MQ,
      JOIN_IN_MQ,
      EVAL_IN_MQ,
      EVAL_OUT_MQ,
      OUTPUT_MQ,
      JOIN_OUT_MQ,                // only used in DMCS-SAT algorithm
      END_OF_MQ
    };

  NewConcurrentMessageDispatcher(std::size_t k, std::size_t no_neighbors = 0);
  
  void
  registerMQ(ConcurrentMessageQueuePtr mq, MQIDs id);
  
  void
  registerMQ(ConcurrentMessageQueuePtr mq, MQIDs type, std::size_t id);

  std::size_t
  createAndRegisterMQ(MQIDs type);

  template<typename MessageType>
  bool
  send(MQIDs id, MessageType* mess, int msecs);

  template<typename MessageType>
  bool
  send(MQIDs type, std::size_t id, MessageType* mess, int msecs);

  template<typename MessageType>
  MessageType*
  receive(MQIDs id, int msecs);

  template<typename MessageType>
  MessageType*
  receive(MQIDs type, std::size_t id, int msecs);

private:
  void
  init_mqs(std::size_t k, std::size_t no_neighbors);

  ConcurrentMessageQueuePtr
  getMQ(MQIDs id);

  ConcurrentMessageQueuePtr
  getMQ(MQIDs type, std::size_t id);
  
  template<typename MessageType>
  bool
  send(ConcurrentMessageQueue* cmq, MessageType* mess, int msecs);

  template<typename MessageType>
  MessageType*
  receive(ConcurrentMessageQueue* cmq, int msecs);

private:
  ConcurrentMessageQueueVec2 cmqs;
};

typedef boost::shared_ptr<NewConcurrentMessageDispatcher> NewConcurrentMessageDispatcherPtr;

} // namespace dmcs

#include "network/NewConcurrentMessageDispatcher.tcc"

#endif // _CONCURRENT_MESSAGE_DISPATCHER_H

// Local Variables:
// mode: C++
// End:

