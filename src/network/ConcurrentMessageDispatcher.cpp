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
 * @file   ConcurrentMessageDispatcher.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Jan  9 08:44:14 2011
 * 
 * @brief  
 * 
 * 
 */


#include "mcs/BeliefState.h"
#include "solver/Conflict.h"
#include "network/ConcurrentMessageDispatcher.h"

using namespace dmcs;

ConcurrentMessageDispatcher::ConcurrentMessageDispatcher()
{ }


ConcurrentMessageDispatcher::~ConcurrentMessageDispatcher()
{
  // no cleanup needed
#if 0
  // remove all registered message queues
  for (std::vector<std::string>::const_iterator nm = nms.begin();
       nm != nms.end();
       ++nm)
    {
      MQ::remove(nm->c_str());
    }
#endif
}


void
ConcurrentMessageDispatcher::registerMQ(ConcurrentMessageQueuePtr& mq, std::size_t id)
{
  ///@todo maybe we want to register boost::weak_ptr<ConcurrentMessageQueue> instead??
  mqs.push_back(mq);
  mqids.push_back(id); // maybe not needed
}


bool
ConcurrentMessageDispatcher::sendModel(PartialBeliefState* b,
				       std::size_t /* from */,
				       std::size_t to,
				       std::size_t /* prio */,
				       int msecs)
{
  ///@todo TK: from and prio are not used
  assert(mqs.size() > to);

  if (msecs > 0)
    {
      return mqs[to]->timed_send(&b, sizeof(b), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      return mqs[to]->try_send(&b, sizeof(b), 0);
    }
  else
    {
      mqs[to]->send(&b, sizeof(b), 0);
      return true;
    }
}


bool
ConcurrentMessageDispatcher::sendConflict(Conflict* c,
					  std::size_t /* from */,
					  std::size_t to,
					  std::size_t /* prio */,
					  int msecs)
{
  ///@todo TK: from and prio are not used
  assert(mqs.size() > to);

  if (msecs > 0)
    {
      return mqs[to]->timed_send(&c, sizeof(c), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      return mqs[to]->try_send(&c, sizeof(c), 0);
    }
  else
    {
      mqs[to]->send(&c, sizeof(c), 0);
      return true;
    }
}


bool
ConcurrentMessageDispatcher::sendModelConflict(PartialBeliefState* b,
					       Conflict* c,
					       std::size_t /* from */,
					       std::size_t to,
					       std::size_t /* prio */,
					       int msecs)
{
  ///@todo TK: from and prio are not used
  assert(mqs.size() > to);

  struct ModelConflict mc = { b, c };

  if (msecs > 0)
    {
      return mqs[to]->timed_send(&mc, sizeof(mc), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      return mqs[to]->try_send(&mc, sizeof(b), 0);
    }
  else
    {
      mqs[to]->send(&mc, sizeof(mc), 0);
      return true;
    }
}


bool
ConcurrentMessageDispatcher::sendJoinIn(std::size_t k,
					std::size_t from,
					std::size_t to,
					std::size_t /* prio */,
					int msecs)
{
  ///@todo TK: prio is not used
  assert(mqs.size() > to);
  assert(mqs.size() > from);

  struct JoinIn ji = { from, k };

  if (msecs > 0)
    {
      return mqs[to]->timed_send(&ji, sizeof(ji), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      return mqs[to]->try_send(&ji, sizeof(ji), 0);
    }
  else
    {
      mqs[to]->send(&ji, sizeof(ji), 0);
      return true;
    }
}


PartialBeliefState*
ConcurrentMessageDispatcher::recvModel(std::size_t from,
				       std::size_t& /* prio */,
				       int& msecs)
{
  ///@todo TK: prio is not used
  assert(mqs.size() > from);

  PartialBeliefState* b = 0;
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&b);

  if (msecs > 0)
    {
      if (!mqs[from]->timed_receive(ptr, sizeof(b), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  return 0;
	}
    }
  else if (msecs < 0)
    {
      if (!mqs[from]->try_receive(ptr, sizeof(b), recvd, p))
	{
	  msecs = 0;
	  return 0;
	}
    }
  else
    {
      mqs[from]->receive(ptr, sizeof(b), recvd, p);
    }

  assert(sizeof(b) == recvd);

  //prio = p;
  return b;
}


Conflict*
ConcurrentMessageDispatcher::recvConflict(std::size_t from,
					  std::size_t& /* prio */,
					  int& msecs)
{
  ///@todo TK: prio is not used
  assert(mqs.size() > from);

  Conflict* c = 0;
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&c);

  if (msecs > 0)
    {
      if (!mqs[from]->timed_receive(ptr, sizeof(c), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  return 0;
	}
    }
  else if (msecs < 0)
    {
      if (!mqs[from]->try_receive(ptr, sizeof(c), recvd, p))
	{
	  msecs = 0;
	  return 0;
	}
    }
  else
    {
      mqs[from]->receive(ptr, sizeof(c), recvd, p);
    }

  assert(sizeof(c) == recvd);

  //prio = p;
  return c;
}


struct MessagingGateway<PartialBeliefState, Conflict>::ModelConflict
ConcurrentMessageDispatcher::recvModelConflict(std::size_t from,
					       std::size_t& /* prio */,
					       int& msecs)
{
  assert(mqs.size() > from);

  struct ModelConflict mc = {0,0};
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&mc);


  if (msecs > 0)
    {
      if (!mqs[from]->timed_receive(ptr, sizeof(mc), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  return mc;
	}
    }
  else if (msecs < 0)
    {
      if (!mqs[from]->try_receive(ptr, sizeof(mc), recvd, p))
	{
	  msecs = 0;
	  return mc;
	}
    }
  else
    {
      mqs[from]->receive(ptr, sizeof(mc), recvd, p);
    }

  assert(sizeof(mc) == recvd);

  //prio = p;
  return mc;
}


struct MessagingGateway<PartialBeliefState, Conflict>::JoinIn
ConcurrentMessageDispatcher::recvJoinIn(std::size_t from,
					std::size_t& /* prio */,
					int& msecs)
{
  assert(mqs.size() > from);

  struct JoinIn ji = {0,0};
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&ji);

  if (msecs > 0)
    {
      if (!mqs[from]->timed_receive(ptr, sizeof(ji), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  return ji;
	}
    }
  else if (msecs < 0)
    {
      if (!mqs[from]->try_receive(ptr, sizeof(ji), recvd, p))
	{
	  msecs = 0;
	  return ji;
	}
    }
  else
    {
      mqs[from]->receive(ptr, sizeof(ji), recvd, p);
    }

  assert(sizeof(ji) == recvd);

  //prio = p;
  return ji;
}


// Local Variables:
// mode: C++
// End:
