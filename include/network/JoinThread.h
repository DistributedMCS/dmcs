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

#include "bm/bm.h"

#include "mcs/BeliefState.h"
#include "mcs/HashedBiMap.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "solver/Conflict.h"

namespace dmcs {

class JoinThread
{
public:
  JoinThread(std::size_t no_nbs_,
	     const HashedBiMapPtr& c2o_,
	     boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_);

  void 
  operator()();

private:
  void
  import_belief_states(std::size_t ctx_id, std::size_t peq_cnt,
		       BeliefStatePackagePtr partial_eqs, 
		       bm::bvector<>& in_mask,
		       bm::bvector<>& end_mask,
		       BeliefStateIteratorVecPtr beg_it, 
		       BeliefStateIteratorVecPtr mid_it,
		       bool first_import);

  std::size_t
  join(const BeliefStateIteratorVecPtr run_it);

  void
  join(BeliefStatePackagePtr partial_eqs, 
       BeliefStateIteratorVecPtr beg_it, 
       BeliefStateIteratorVecPtr mid_it);

private:
  std::size_t           no_nbs;      // number of neighbors left from which I need the models
  HashedBiMapPtr        c2o;
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;
};

} // namespace dmcs


#endif // JOIN_THREAD_H

// Local Variables:
// mode: C++
// End:
