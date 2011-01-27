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

#include "mcs/BeliefState.h"
#include "mcs/HashedBiMap.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "solver/Conflict.h"

#include <boost/thread.hpp>
#include <boost/thread/future.hpp>

namespace dmcs {

class JoinThread
{
public:
  JoinThread();

  virtual
  ~JoinThread();

  void 
  operator()(std::size_t no_nbs,
	     std::size_t system_size,
	     MessagingGatewayBC* mg,
	     ConcurrentMessageQueueVec* joiner_neighbors_notif);


  enum ImportStates
    {
      START_UP = 0,   // the very fist start. We need this in order to distinguish between UNSAT and EOF
      FILLING_UP,     // the state where not all models have returned the full package of models
      GETTING_NEXT    // next round, meaning that we reached PACK_FULL before and now want to ask a neighbor for its next models
    };

private:
  bool
  import_belief_states(std::size_t noff,
		       std::size_t peq_cnt,
		       PartialBeliefStatePackagePtr& partial_eqs, 
		       bm::bvector<>& in_mask,
		       bm::bvector<>& end_mask,
		       PartialBeliefStateIteratorVecPtr& beg_it, 
		       PartialBeliefStateIteratorVecPtr& mid_it,
		       ImportStates import_state);

  void
  ask_for_next(PartialBeliefStatePackagePtr& partial_eqs, 
	       std::size_t next);

  std::size_t
  join(const PartialBeliefStateIteratorVecPtr& run_it);

  void
  join(const PartialBeliefStatePackagePtr& partial_eqs, 
       const PartialBeliefStateIteratorVecPtr& beg_it, 
       const PartialBeliefStateIteratorVecPtr& end_it);

private:
  std::size_t                  no_nbs;                  // number of neighbors 
  std::size_t                  system_size;

  MessagingGatewayBC*        mg;
  ConcurrentMessageQueueVec* joiner_neighbors_notif;
};

} // namespace dmcs

#endif // JOIN_THREAD_H

// Local Variables:
// mode: C++
// End:
