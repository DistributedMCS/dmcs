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
 * @file   JoinThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:00:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef JOIN_THREAD_H
#define JOIN_THREAD_H

#include <bm/bm.h>

#include "dmcs/ConflictNotification.h"
#include "dmcs/ModelSessionId.h"
#include "mcs/BeliefState.h"
#include "mcs/HashedBiMap.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/JoinerDispatcher.h"

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

namespace dmcs {

class JoinThread
{
public:
  JoinThread(std::size_t p,
	     std::size_t cid,
	     std::size_t sid,
	     JoinerDispatcher* jd);

  virtual
  ~JoinThread();

  void 
  operator()(std::size_t no_nbs,
	     std::size_t system_size,
	     MessagingGatewayBC* mg,
	     ConcurrentMessageQueue* jsn,
	     ConcurrentMessageQueue* sjn,
	     ConcurrentMessageQueueVec* joiner_neighbors_notif);


  enum ImportStates
    {
      START_UP = 0,   // the very first start. We need this in order to distinguish between UNSAT and EOF
      FILLING_UP,     // the state where not all models have returned the full package of models
      GETTING_NEXT    // next round, meaning that we reached PACK_FULL before and now want to ask a neighbor for its next models
    };

  ConcurrentMessageQueue*
  getCMQ();

private:
  bool
  join(const PartialBeliefStateIteratorVecPtr& run_it);

  bool
  join(const PartialBeliefStatePackagePtr& partial_eqs, 
       const PartialBeliefStateIteratorVecPtr& beg_it, 
       const PartialBeliefStateIteratorVecPtr& end_it);

  void
  first_join(std::size_t path, 
	     std::size_t session_id,
	     std::size_t k_one, 
	     std::size_t k_two,
	     bool& first_round,
	     bool& asking_next,
	     std::size_t& next_neighbor_offset,
	     VecSizeTPtr& pack_count,
	     PartialBeliefStatePackagePtr& partial_eqs);

  void
  next_join(std::size_t path, 
	    std::size_t session_id,
	    std::size_t k_one, 
	    std::size_t k_two,
	    bool& first_round,
	    bool& asking_next,
	    std::size_t& next_neighbor_offset,
	    VecSizeTPtr& pack_count,
	    PartialBeliefStatePackagePtr& partial_eqs);

  void
  cleanup_partial_belief_states(PartialBeliefStatePackage* partial_eqs, 
				std::size_t nid);


  // only ask and let other methods take care of the receiving
  void
  ask_neighbor(PartialBeliefStatePackage* partial_eqs,
	       std::size_t noff, 
	       std::size_t k1, 
	       std::size_t k2,
	       std::size_t& path, 
	       BaseNotification::NotificationType nt);

  // ask and then wait for the neighbor to return the models
  bool
  ask_neighbor_and_receive(PartialBeliefStatePackage* partial_eqs,
			   std::size_t noff, 
			   std::size_t k1, 
			   std::size_t k2,
			   std::size_t& path, 
			   BaseNotification::NotificationType nt);

  bool
  ask_first_packs(PartialBeliefStatePackage* partial_eqs,
		  std::size_t& path, 
		  std::size_t from_neighbor, 
		  std::size_t to_neighbor);


  bool
  do_join(PartialBeliefStatePackagePtr& partial_eqs);

  void
  process(std::size_t path, 
	  std::size_t session_id,
	  std::size_t k1, 
	  std::size_t k2,
	  bool& first_round,
	  bool& asking_next,
	  std::size_t& neighbor_offset,
	  VecSizeTPtr& pack_count,
	  PartialBeliefStatePackagePtr& partial_eqs);

  void
  reset(bool& first_round,
	bool& asking_next,
	std::size_t next_neighbor_offset,
	PartialBeliefStatePackagePtr& partial_eqs,
	VecSizeTPtr& pack_count);

private:
  std::size_t port;
  std::size_t no_nbs;            // number of neighbors 
  std::size_t system_size;
  std::size_t session_id;
  std::size_t pack_size;         // the real upper-bound of number of models that we ask the neighbors
  std::size_t path;
  std::size_t ctx_id;
  bool first_result;
  ModelSessionIdListPtr joined_results;

  MessagingGatewayBC* mg;
  ConcurrentMessageQueue* joiner_sat_notif; // to notify SAT of joined results
  ConcurrentMessageQueue* sat_joiner_notif; // to get notifications from SAT regarding next request
  ConcurrentMessageQueueVec* joiner_neighbors_notif;
  JoinerDispatcher* joiner_dispatcher;
  ConcurrentMessageQueuePtr input_queue;
};

} // namespace dmcs

#endif // JOIN_THREAD_H

// Local Variables:
// mode: C++
// End:
