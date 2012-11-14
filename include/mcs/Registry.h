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

#include <set>

#include "network/NewConcurrentMessageDispatcher.h"
#include "mcs/BeliefStateOffset.h"
#include "mcs/NormalContext.h"
#include "network/NewOutputDispatcher.h"
#include "network/NewJoinerDispatcher.h"
#include "network/RequestDispatcher.h"
#include "mcs/NewNeighbor.h"

namespace dmcs {

struct Registry
{
  Registry(const std::size_t ss,
	   const std::size_t qs,
	   const std::size_t bs,
	   const std::string& manager_hostname,
	   const std::string& manager_port,
	   NormalContextVecPtr cs)
    : system_size(ss),
      queue_size(qs),
      belief_set_size(bs),
      manager_hostname(manager_hostname),
      manager_port(manager_port),
      belief_state_offset(0),
      contexts(cs)
  {
    std::cerr << "Init Registry" << std::endl;
    std::set<NewNeighbor*> tmp_storage;
    neighbors = boost::shared_ptr<NewNeighborVec>(new NewNeighborVec);
    bool has_neighbors = false;

    for (NormalContextVec::const_iterator it = contexts->begin(); it != contexts->end(); ++it)
      {
	NormalContextPtr ctx = *it;
	NewNeighborVecPtr ctx_neighbors = ctx->getNeighbors();
	if (ctx_neighbors)
	  {
	    for (NewNeighborVec::const_iterator jt = ctx_neighbors->begin(); jt != ctx_neighbors->end(); ++jt)
	      {
		NewNeighborPtr neighbor = *jt;
		std::set<NewNeighbor*>::const_iterator kt = tmp_storage.find(neighbor.get());
		if (kt == tmp_storage.end())
		  {
		    tmp_storage.insert(neighbor.get());
		    neighbors->push_back(neighbor);
		    has_neighbors = true;
		  }
	      }
	  }
      }

    if (!has_neighbors)	neighbors.reset();
  }

  Registry(const std::size_t ss,
	   const std::size_t qs,
	   const std::size_t bs,
	   const std::string& manager_hostname,
	   const std::string& manager_port,
	   NormalContextVecPtr cs,
	   NewNeighborVecPtr ns)
    : system_size(ss),
      queue_size(qs),
      belief_set_size(bs),
      manager_hostname(manager_hostname),
      manager_port(manager_port),
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
  std::string manager_hostname;
  std::string manager_port;

  BeliefStateOffset* belief_state_offset;
  NewConcurrentMessageDispatcherPtr message_dispatcher;
  RequestDispatcherPtr request_dispatcher;
  NewOutputDispatcherPtr output_dispatcher;
  NewJoinerDispatcherPtr joiner_dispatcher;
  NormalContextVecPtr contexts;
  NewNeighborVecPtr neighbors;
};

typedef boost::shared_ptr<Registry> RegistryPtr;

} // namespace dmcs

#endif // REGISTRY_H

// Local Variables:
// mode: C++
// End:
