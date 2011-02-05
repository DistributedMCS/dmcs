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
				       std::size_t sid,
				       std::size_t /* from */,
				       std::size_t to,
				       std::size_t /* prio */,
				       int msecs)
{
  ///@todo TK: from and prio are not used
  assert(mqs.size() > to);

  struct ModelSession ms = { b, sid };

  if (msecs > 0)
    {
      return mqs[to]->timed_send(&ms, sizeof(ms), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      return mqs[to]->try_send(&ms, sizeof(ms), 0);
    }
  else
    {
      mqs[to]->send(&ms, sizeof(ms), 0);
      return true;
    }
}



#if 0
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
      return mqs[to]->try_send(&mc, sizeof(mc), 0);
    }
  else
    {
      mqs[to]->send(&mc, sizeof(mc), 0);
      return true;
    }
}


bool
ConcurrentMessageDispatcher::sendModelDecisionlevel(PartialBeliefState* b,
						    Decisionlevel* d,
						    std::size_t /* from */,
						    std::size_t to,
						    std::size_t /* prio */,
						    int msecs)
{
  ///@todo TK: from and prio are not used
  assert(mqs.size() > to);

  struct ModelDecisionlevel md = { b, d };

  if (msecs > 0)
    {
      return mqs[to]->timed_send(&md, sizeof(md), 0, boost::posix_time::milliseconds(msecs));
    }
  else if (msecs < 0)
    {
      return mqs[to]->try_send(&md, sizeof(md), 0);
    }
  else
    {
      mqs[to]->send(&md, sizeof(md), 0);
      return true;
    }
}
#endif//0


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


struct MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::ModelSession
ConcurrentMessageDispatcher::recvModel(std::size_t from,
				       std::size_t& /* prio */,
				       int& msecs)
{
  ///@todo TK: prio is not used
  assert(mqs.size() > from);

  struct ModelSession ms = {0,0};
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&ms);

  if (msecs > 0)
    {
      if (!mqs[from]->timed_receive(ptr, sizeof(ms), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  ms.m = 0;
	  ms.sid = 0;
	  return ms;
	}
    }
  else if (msecs < 0)
    {
      if (!mqs[from]->try_receive(ptr, sizeof(ms), recvd, p))
	{
	  msecs = 0;
	  ms.m = 0;
	  ms.sid = 0;
	  return ms;
	}
    }
  else
    {
      mqs[from]->receive(ptr, sizeof(ms), recvd, p);
    }

  assert(sizeof(ms) == recvd);

  //prio = p;
  return ms;
}


#if 0
struct MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::ModelConflict
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


struct MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::ModelDecisionlevel
ConcurrentMessageDispatcher::recvModelDecisionlevel(std::size_t from,
						    std::size_t& /* prio */,
						    int& msecs)
{
  assert(mqs.size() > from);

  struct ModelDecisionlevel md = {0,0};
  std::size_t recvd = 0;
  unsigned int p = 0;
  void *ptr = static_cast<void*>(&md);


  if (msecs > 0)
    {
      if (!mqs[from]->timed_receive(ptr, sizeof(md), recvd, p, boost::posix_time::milliseconds(msecs)))
	{
	  msecs = 0;
	  return md;
	}
    }
  else if (msecs < 0)
    {
      if (!mqs[from]->try_receive(ptr, sizeof(md), recvd, p))
	{
	  msecs = 0;
	  return md;
	}
    }
  else
    {
      mqs[from]->receive(ptr, sizeof(md), recvd, p);
    }

  assert(sizeof(md) == recvd);

  //prio = p;
  return md;
}
#endif//0


struct MessagingGateway<PartialBeliefState, Decisionlevel, Conflict>::JoinIn
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
