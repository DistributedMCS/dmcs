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
 * @file   BaseDispatcher.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Aug  16 10:00:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef BASE_DISPATCHER_H
#define BASE_DISPATCHER_H

#include "network/ConcurrentMessageQueueFactory.h"
#include "network/MessagingGateway.h"

namespace dmcs {

class BaseDispatcher
{
public:
  BaseDispatcher()
    : thread_map(new CMQMap) 
  { }

  ~BaseDispatcher()
  { }

  void
  registerThread(std::size_t path, ConcurrentMessageQueue* cmq)
  {
    // make sure that if this thread was registered then it must be cmq
    CMQMap::const_iterator it = thread_map->find(path);
    if (it != thread_map->end())
      {
	assert (cmq == it->second);
      }
    else
      // register the new thread
      {
	std::pair<std::size_t, ConcurrentMessageQueue*> t(path, cmq);
	thread_map->insert(t);
      }
  }

  virtual void
  operator()(MessagingGatewayBC* mg) = 0;
  
protected:
  CMQMapPtr thread_map;
};


} // namespace dmcs

#endif // BASE_DISPATCHER_H

// Local Variables:
// mode: C++
// End:
