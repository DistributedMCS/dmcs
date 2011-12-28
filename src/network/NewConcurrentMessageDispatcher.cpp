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

NewConcurrentMessageDispatcher::NewConcurrentMessageDispatcher()
{
  std::size_t n = JOIN_OUT_MQ - SEPARATOR;
  for (std::size_t i = 0; i < n; ++i)
    {
      ConcurrentMessageQueueVecPtr v(new ConcurrentMessageQueueVec);
      cmqs.push_back(v);
    }

  std::size_t m = SEPARATOR - 1;
  ConcurrentMessageQueueVecPtr& v = cmqs[0];
  v->resize(m);
}



void
NewConcurrentMessageDispatcher::registerMQ(ConcurrentMessageQueuePtr& mq, MQIDs id)
{
  assert ( id < SEPARATOR );
  ConcurrentMessageQueueVecPtr& v = cmqs[0];
  (*v)[id] = mq;
}



void
NewConcurrentMessageDispatcher::registerMQ(ConcurrentMessageQueuePtr& mq, MQIDs type, std::size_t id)
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


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
