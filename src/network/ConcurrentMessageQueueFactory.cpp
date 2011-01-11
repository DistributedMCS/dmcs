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
 * @file   ConcurrentMessageQueueFactory.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Jan  9 09:29:58 2011
 * 
 * @brief  
 * 
 * 
 */

#include "network/ConcurrentMessageQueueFactory.h"
#include "network/ConcurrentMessageDispatcher.h"
#include "network/ConcurrentMessageQueue.h"

#include <sstream>

using namespace dmcs;


/// boost singleton
boost::scoped_ptr<ConcurrentMessageQueueFactory> ConcurrentMessageQueueFactory::ins(0);
boost::once_flag ConcurrentMessageQueueFactory::flag = BOOST_ONCE_INIT;


ConcurrentMessageQueueFactory&
ConcurrentMessageQueueFactory::instance()
{
  // expensive, but clients should cache the factory anyways...
  boost::call_once(init, flag);
  return *ins;
}


void
ConcurrentMessageQueueFactory::init() // never throws
{
  ins.reset(new ConcurrentMessageQueueFactory);
}


ConcurrentMessageQueueFactory::ConcurrentMessageQueueFactory()
{ }


ConcurrentMessageQueueFactory::ConcurrentMessageQueueFactory(const ConcurrentMessageQueueFactory&)
  : boost::noncopyable()
{ }


ConcurrentMessageQueueFactory::~ConcurrentMessageQueueFactory()
{
  ///@todo TK: what to do here?
  ///on shutdown, we don't care, as we don't have any persistent resources...
}


ConcurrentMessageQueuePtr
ConcurrentMessageQueueFactory::createMessageQueue(std::size_t id, std::size_t max_k, std::size_t /* max_size */)
{
  ///@todo maybe we should return boost::weak_ptr here

  if (mqs.find(id) != mqs.end())
    {
      return mqs[id];
    }

  ConcurrentMessageQueuePtr mq(new ConcurrentMessageQueue(max_k));

  mqs[id] = mq;

  return mq;
}


boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >
ConcurrentMessageQueueFactory::createMessagingGateway(std::size_t uid, std::size_t no_nbs)
{
  ///@todo TK: all MQs need to have a unique name in the _process_, thus
  ///we need to add the port number of creating session to the id

  boost::shared_ptr<ConcurrentMessageDispatcher> md(new ConcurrentMessageDispatcher);
  ConcurrentMessageQueuePtr mq;

  ///@todo fixme, code duplication as well...
  std::size_t k = 5;


  // create message queue 0
  // conflict input MQ, announces conflicts from the parent

  std::size_t id = uid;
  mq = createMessageQueue(id, k, sizeof(Conflict*));
  md->registerMQ(mq, id);

  // create message queue 1
  // output MQ, announces partial equilibria w.r.t. conflicts
  
  id++;
  mq = createMessageQueue(id, k, sizeof(MessagingGateway<BeliefState,Conflict>::ModelConflict));
  md->registerMQ(mq, id);

  // create message queue 2
  // conflict output MQ, announces new conflicts from the local solver

  id++;
  mq = createMessageQueue(id, k, sizeof(Conflict*));
  md->registerMQ(mq, id);

  // create message queue 3
  // join output MQ, announces joined belief states from the neighbors

  id++;
  mq = createMessageQueue(id, k, sizeof(BeliefState*));
  md->registerMQ(mq, id);

  // create message queue 4
  // join input MQ, announces that neighbor C_i sent k partial belief states

  id++;
  mq = createMessageQueue(id, k, sizeof(MessagingGateway<BeliefState,Conflict>::JoinIn));
  md->registerMQ(mq, id);

  // create message queues 5 to 5+2*no_nbs

  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      // partial equilibrium input MQ for neighbor C_i

      id++;
      mq = createMessageQueue(id, k, sizeof(BeliefState*));
      md->registerMQ(mq, id);

      // conflict input MQ, announces a new conflict to a neighbor C_i

      id++;
      mq = createMessageQueue(id, k, sizeof(Conflict*));
      md->registerMQ(mq, id);
    }

  return md;
}



boost::shared_ptr<MessagingGateway<BeliefState,Conflict> >
ConcurrentMessageQueueFactory::createMessagingGateway(std::size_t uid)
{
  ///@todo TK: all MQs need to have a unique name in the _process_, thus
  ///we need to add the port number of creating session to the id

  boost::shared_ptr<ConcurrentMessageDispatcher> md(new ConcurrentMessageDispatcher);
  ConcurrentMessageQueuePtr mq;

  ///@todo fixme, code duplication as well...
  std::size_t k = 5;

  // create message queue 0
  // conflict input MQ, announces conflicts from the parent

  std::size_t id = uid;
  mq = createMessageQueue(id, k, sizeof(Conflict*));
  md->registerMQ(mq, id);

  // create message queue 1
  // output MQ, announces partial equilibria
  
  id++;
  mq = createMessageQueue(id, k, sizeof(MessagingGateway<BeliefState,Conflict>::ModelConflict));
  md->registerMQ(mq, id);

  return md;
}


// Local Variables:
// mode: C++
// End:
