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
  * @file   StreamingJoiner.h
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Tue Jan  3 14:21:27 2012
  * 
  * @brief  Joiner that returns BeliefState(s) one by one
  * 
  * 
  */

#ifndef STREAMING_JOINER_H
#define STREAMING_JOINER_H

#include <map>
#include <boost/shared_ptr.hpp>

#include "mcs/BaseJoiner.h"
#include "mcs/ReturnedBeliefState.h"

namespace dmcs {

typedef std::map<std::size_t, std::size_t> NeighborOffset2Index;
typedef boost::shared_ptr<NeighborOffset2Index> NeighborOffset2IndexPtr;

class StreamingJoiner : public BaseJoiner
{
public:
  StreamingJoiner(std::size_t c, 
		  NewNeighborVecPtr n,
		  NeighborOffset2IndexPtr o2i);

  ReturnedBeliefState*
  trigger_join(std::size_t query_id, 
	       std::size_t k1, 
	       std::size_t k2,
	       NewConcurrentMessageDispatcherPtr md,
	       NewJoinerDispatcherPtr jd);

private:
  void
  reset();

  ReturnedBeliefState*
  process(std::size_t query_id, 
	  std::size_t k1, 
	  std::size_t k2,
	  NewConcurrentMessageDispatcherPtr md,
	  NewJoinerDispatcherPtr jd);

  ReturnedBeliefState*
  first_join(std::size_t query_id, 
	     std::size_t k1, 
	     std::size_t k2,
	     NewConcurrentMessageDispatcherPtr md,
	     NewJoinerDispatcherPtr jd);

  ReturnedBeliefState*
  next_join(std::size_t query_id, 
	    std::size_t k1, 
	    std::size_t k2,
	    NewConcurrentMessageDispatcherPtr md,
	    NewJoinerDispatcherPtr jd);


  bool
  do_join(std::size_t query_id);


  bool
  join(std::size_t query_id,
       const NewBeliefStateIteratorVecPtr& beg_it, 
       const NewBeliefStateIteratorVecPtr& end_it);

  bool
  join(std::size_t query_id,
       const NewBeliefStateIteratorVecPtr& run_it);

  bool
  ask_first_packs(std::size_t query_id, 
		  std::size_t from_neighbor, 
		  std::size_t to_neighbor,
		  NewConcurrentMessageDispatcherPtr md,
		  NewJoinerDispatcherPtr jd);

  void
  ask_neighbor(std::size_t neighbor_index, 
	       std::size_t query_id, 
	       std::size_t k1, 
	       std::size_t k2,
	       NewConcurrentMessageDispatcherPtr md,
	       NewJoinerDispatcherPtr jd);

  bool
  ask_neighbor_and_receive(std::size_t noff,
			   std::size_t query_id,
			   std::size_t k1,
			   std::size_t k2,
			   NewConcurrentMessageDispatcherPtr md,
			   NewJoinerDispatcherPtr jd);

private:
  std::size_t pack_size;            // the real upper-bound of number of models that we ask the neighbors
  std::size_t next_neighbor;
  bool first_round;
  bool asking_next;
  std::vector<std::size_t> pack_count;
  NeighborOffset2IndexPtr offset2index;
};

typedef boost::shared_ptr<StreamingJoiner> StreamingJoinerPtr;

} // namespace dmcs

#endif // BASE_JOINER_H

// Local Variables:
// mode: C++
// End:
