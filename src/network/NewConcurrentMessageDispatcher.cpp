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
 * @file   NewConcurrentMessageDispatcher.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Wed Dec  28 17:41:27 2011
 * 
 * @brief  
 * 
 * 
 */

#include "network/NewConcurrentMessageDispatcher.h"

namespace dmcs {

NewConcurrentMessageDispatcher::NewConcurrentMessageDispatcher(std::size_t k, std::size_t no_neighbors)
{
  init_mqs(k, no_neighbors);
}



void
NewConcurrentMessageDispatcher::init_mqs(std::size_t k, std::size_t no_neighbors)
{
  ConcurrentMessageQueueVecPtr cmq0(new ConcurrentMessageQueueVec);
  cmq0->resize(SEPARATOR);
  cmqs.push_back(cmq0);

  ConcurrentMessageQueuePtr mq_request_dispatcher(new ConcurrentMessageQueue(k));
  registerMQ(mq_request_dispatcher, REQUEST_DISPATCHER_MQ);

  ConcurrentMessageQueuePtr mq_output_dispatcher(new ConcurrentMessageQueue(k));
  registerMQ(mq_output_dispatcher, OUTPUT_DISPATCHER_MQ);

  ConcurrentMessageQueuePtr mq_joiner_dispatcher(new ConcurrentMessageQueue(k));
  registerMQ(mq_joiner_dispatcher, JOINER_DISPATCHER_MQ); 

  std::size_t n = JOIN_OUT_MQ - SEPARATOR;
  for (std::size_t i = 0; i < n; ++i)
    {
      ConcurrentMessageQueueVecPtr v(new ConcurrentMessageQueueVec);
      cmqs.push_back(v);  
    }

  for (std::size_t i = 0; i < no_neighbors; ++i)
    {
      ConcurrentMessageQueuePtr mq_neighbor_in(new ConcurrentMessageQueue(k));
      registerMQ(mq_neighbor_in, NEIGHBOR_IN_MQ, i);

      ConcurrentMessageQueuePtr mq_neighbor_out(new ConcurrentMessageQueue(k));
      registerMQ(mq_neighbor_out, NEIGHBOR_OUT_MQ, i);
    }
}



void
NewConcurrentMessageDispatcher::registerMQ(ConcurrentMessageQueuePtr mq, MQIDs id)
{
  assert ( id < SEPARATOR );
  ConcurrentMessageQueueVecPtr& v = cmqs[0];
  (*v)[id] = mq;
}



void
NewConcurrentMessageDispatcher::registerMQ(ConcurrentMessageQueuePtr mq, MQIDs type, std::size_t id)
{
  assert (SEPARATOR < type && type < END_OF_MQ);

  std::size_t type_index = type - SEPARATOR;
  ConcurrentMessageQueueVecPtr& v = cmqs[type_index];
  
  if (id < v->size())
    {
      (*v)[id] = mq;
    }
  else if (id == v->size())
    {
      v->push_back(mq);
    }
  else
    {
      v->resize(id+1);
      (*v)[id] = mq;
    }
}



ConcurrentMessageQueuePtr
NewConcurrentMessageDispatcher::getMQ(MQIDs id)
{
  assert (id < SEPARATOR);
  ConcurrentMessageQueueVecPtr& v = cmqs[0];
  return (*v)[id];
}



ConcurrentMessageQueuePtr
NewConcurrentMessageDispatcher::getMQ(MQIDs type, std::size_t id)
{
  assert (SEPARATOR < type && type < END_OF_MQ);

  std::size_t type_index = type - SEPARATOR;
  ConcurrentMessageQueueVecPtr& v = cmqs[type_index];

  assert (id < v->size());
  return (*v)[id];
}


std::size_t
NewConcurrentMessageDispatcher::createAndRegisterMQ(MQIDs type)
{
  assert (SEPARATOR < type && type < END_OF_MQ);

  std::size_t type_index = type - SEPARATOR;
  ConcurrentMessageQueueVecPtr& v = cmqs[type_index];
  ConcurrentMessageQueuePtr mq(new ConcurrentMessageQueue);

  ///TODO: more intelligent way of managing ConcurrentMessageQueue inside the MessageDispatcher:
  ///For example: MD maintains a list of unused cmq(s) and when the client requests a cmq, it just
  ///             find one in the list, marks it somehow as used and return the id to the client.
  ///For now, we use this stupid strategy which might waste memory.
  v->push_back(mq);
  return v->size() - 1;
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
