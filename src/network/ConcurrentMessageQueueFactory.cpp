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
ConcurrentMessageQueueFactory::createMessageQueue(std::size_t uid,
						  std::size_t no_nbs,
						  std::size_t off,
						  std::size_t max_k,
						  std::size_t /* max_size */)
{
  boost::mutex::scoped_lock lock(mtx);

  ///@todo maybe we should return boost::weak_ptr here

  typedef std::map<std::size_t,std::vector<ConcurrentMessageQueuePtr> > mapt;

  mapt::iterator it = mqs.find(uid);

  if (it != mqs.end())
    {
      if (off < it->second.size())
	{
	  if (it->second[off] != 0) // uid and off
	    {
	      return it->second[off];
	    }
	  else // uid and no off
	    {
	      ConcurrentMessageQueuePtr mq(new ConcurrentMessageQueue(max_k));
	      it->second[off] = mq;
	      return mq;
	    }
	}
      else // off > size of mq vector
	{
	  assert (off < NEIGHBOR_MQ + no_nbs);

	  it->second.resize(NEIGHBOR_MQ + no_nbs);

	  ConcurrentMessageQueuePtr mq(new ConcurrentMessageQueue(max_k));
	  it->second[off] = mq;
	  return mq;
	}
    }
  else // no uid and no off
    {
      mapt::value_type val = std::make_pair(uid,std::vector<ConcurrentMessageQueuePtr>(NEIGHBOR_MQ + no_nbs));
      std::pair<mapt::iterator,bool> p = mqs.insert(val);

      ConcurrentMessageQueuePtr mq(new ConcurrentMessageQueue(max_k));
      p.first->second[off] = mq;
      return mq;
    }
}


MessagingGatewayBCPtr
ConcurrentMessageQueueFactory::createMessagingGateway(std::size_t uid, std::size_t no_nbs, std::size_t k)
{
  ///@todo TK: all MQs need to have a unique name in the _process_, thus
  ///we need to add the port number of creating session to the id

  boost::shared_ptr<ConcurrentMessageDispatcher> md(new ConcurrentMessageDispatcher);
  ConcurrentMessageQueuePtr mq;

  // create message queue 0 (OUT_MQ)
  // output MQ, announces partial equilibria from the local solver
  
  mq = createMessageQueue(uid, no_nbs, OUT_MQ, k, sizeof(PartialBeliefState*));
  md->registerMQ(mq, OUT_MQ);

  // create message queue 1 (JOIN_OUT_MQ)
  // join output MQ, announces joined belief states from the neighbors

  mq = createMessageQueue(uid, no_nbs, JOIN_OUT_MQ, k, sizeof(MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::ModelSession));
  md->registerMQ(mq, JOIN_OUT_MQ);

  // create message queue 2 (JOIN_IN_MQ)
  // join input MQ, announces pairs of (neighbor_id, partial belief states)

  mq = createMessageQueue(uid, no_nbs, JOIN_IN_MQ, k, sizeof(MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::JoinIn));
  md->registerMQ(mq, JOIN_IN_MQ);

  // create message queues 3 to 3 + (no_nbs - 1)
  // NEIGHBOR_MQ --> NEIGHBOR_MQ + (no_nbs - 1)

  for (std::size_t i = NEIGHBOR_MQ; i < NEIGHBOR_MQ + no_nbs; ++i)
    {
      // partial equilibria MQ, announces a new partial equilibria from a neighbor C_i

      // NEIGHBOR_MQ + noff
      // noff starts from 0
      mq = createMessageQueue(uid, no_nbs, i, k, sizeof(PartialBeliefState*));
      md->registerMQ(mq, i);
    }

  return md;
}



MessagingGatewayBCPtr
ConcurrentMessageQueueFactory::createMessagingGateway(std::size_t uid, std::size_t k)
{
  ///@todo TK: all MQs need to have a unique name in the _process_, thus
  ///we need to add the port number of creating session to the id

  boost::shared_ptr<ConcurrentMessageDispatcher> md(new ConcurrentMessageDispatcher);
  ConcurrentMessageQueuePtr mq;

  // create message queue 0 (OUT_MQ)
  // output MQ, announces partial equilibria
  
  mq = createMessageQueue(uid, 0, OUT_MQ, k, sizeof(PartialBeliefState*));
  md->registerMQ(mq, OUT_MQ);

  // create message queue 1 (JOIN_OUT_MQ)
  // join output MQ, announces joined belief states from the neighbors
  
  mq = createMessageQueue(uid, 0, JOIN_OUT_MQ, k, sizeof(MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::ModelSession));
  md->registerMQ(mq, JOIN_OUT_MQ);

  return md;
}


// Local Variables:
// mode: C++
// End:
