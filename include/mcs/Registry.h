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
 * @file   Registry.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Mar  12 17:41:20 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef REGISTRY_H
#define REGISTRY_H

#include "network/NewConcurrentMessageDispatcher.h"
#include "mcs/BeliefStateOffset.h"
#include "mcs/NewContext.h"
#include "mcs/NewOutputDispatcher.h"
#include "mcs/NewJoinerDispatcher.h"
#include "mcs/RequestDispatcher.h"
#include "mcs/NewNeighbor.h"

namespace dmcs {

struct Registry
{
  Registry(const std::size_t ss,
	   const std::size_t qs,
	   const std::size_t bs,
	   NewContextVecPtr cs)
    : system_size(ss),
      queue_size(qs),
      belief_set_size(bs),
      belief_state_offset(0),
      contexts(cs),
      neighbors(NewNeighborVecPtr())
  { }

  Registry(const std::size_t ss,
	   const std::size_t qs,
	   const std::size_t bs,
	   NewContextVecPtr cs,
	   NewNeighborVecPtr ns)
    : system_size(ss),
      queue_size(qs),
      belief_set_size(bs),
      belief_state_offset(0),
      contexts(cs),
      neighbors(ns)
  { }

  ~Registry()
  {
    if (belief_state_offset)
      {
	delete belief_state_offset;
	belief_state_offset = 0;
      }
  }

  std::size_t system_size;
  std::size_t queue_size;
  std::size_t belief_set_size;

  BeliefStateOffset* belief_state_offset;
  NewConcurrentMessageDispatcherPtr message_dispatcher;
  RequestDispatcherPtr request_dispatcher;
  NewOutputDispatcherPtr output_dispatcher;
  NewJoinerDispatcherPtr joiner_dispatcher;
  NewContextVecPtr contexts;
  NewNeighborVecPtr neighbors;
};

typedef boost::shared_ptr<Registry> RegistryPtr;

} // namespace dmcs

#endif // REGISTRY_H

// Local Variables:
// mode: C++
// End:
