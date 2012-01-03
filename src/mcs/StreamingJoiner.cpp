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
  * @file   StreamingJoiner.cpp
  * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
  * @date   Tue Jan  3 14:24:15 2012
  * 
  * @brief  
  * 
  * 
  */

#include "mcs/BeliefStateOffset.h"
#include "mcs/ForwardMessage.h"
#include "mcs/StreamingJoiner.h"

namespace dmcs {

StreamingJoiner::StreamingJoiner(std::size_t c, std::size_t n,
				 NewConcurrentMessageDispatcherPtr m,
				 NewJoinerDispatcherPtr jd)
  : BaseJoiner(c, n, m, jd),
    next_neighbor_offset(0),
    first_round(true),
    asking_next(false),
    pack_count(n, 0)
{
  init();
}



void
StreamingJoiner::reset()
{
  BaseJoiner::reset();

  first_round = true;
  asking_next = false;
  next_neighbor_offset = 0;
  
  // reset counter
  std::fill(pack_count.begin(), pack_count.end(), 0);
}



ReturnedBeliefState*
StreamingJoiner::trigger_join(std::size_t query_id, std::size_t k1, std::size_t k2)
{
  if (shutdown(query_id))
    {
      reset();
      return NULL;
    }

  if (!joined_results.empty())
    {
      ReturnedBeliefState* rbs = joined_results.front();
      joined_results.pop_front();
      return rbs;
    }
  else
    {
      return process(query_id, k1, k2);
    }
}



ReturnedBeliefState*
StreamingJoiner::process(std::size_t query_id, std::size_t k1, std::size_t k2)
{
  if (first_round)
    {
      return first_join(query_id, k1, k2);
    }
  else if (k2 > 0)
    {
      return next_join(query_id, k1, k2);
    }
  else
    {
      reset();
      return NULL;
    }
}



ReturnedBeliefState*
StreamingJoiner::first_join(std::size_t query_id, std::size_t k1, std::size_t k2)
{
  if (k2 == 0)
    {
      pack_size = 0;
    }
  else
    {
      pack_size = k2 - k1 + 1;
    }

  // Warming up round, set first_round to FALSE
  first_round = false;
  if (!ask_first_packs(query_id, 0, no_neighbors-1))
    {
      // A neighbor is inconsistent. Reset and return NULL 
      reset();
      return NULL;
    }
  
  bool succeeded = do_join(query_id);
  next_neighbor_offset = 0;
  asking_next = false;

  if (succeeded)
    {
      ReturnedBeliefState* rbs = joined_results.front();
      joined_results.pop_front();
      return rbs;
    }

  if (k2 > 0)
    {
      succeeded = next_join(query_id, k1, k2);
      if (succeeded)
	{
	  ReturnedBeliefState* rbs = joined_results.front();
	  joined_results.pop_front();
	  return rbs;
	}
    }

  return NULL;
}



ReturnedBeliefState*
StreamingJoiner::next_join(std::size_t query_id, std::size_t k1, std::size_t k2)
{
  if (k2 == 0)
    {
      pack_size = 0;
    }
  else
    {
      pack_size = k2 - k1 + 1;
    }

  // now really going to the loop of asking next =============================================
  while (1)
    {
      if (next_neighbor_offset == no_neighbors)
	{
	  // No more models from my neighbors	    
	  reset();

	  return NULL;
	}
      
      std::size_t& pc = pack_count[next_neighbor_offset];
      pc++;
      std::size_t k_one = pc * pack_size + 1;
      std::size_t k_two = (pc+1) * pack_size;
      
      // Ask next at neighbor_offset
      if (ask_neighbor_and_receive(next_neighbor_offset, query_id, k_one, k_two))
	{
	  if (asking_next)
	    {
	      assert (next_neighbor_offset > 0);
	            
	      // Ask first packs before neighbor_offset
	      bool ret = ask_first_packs(query_id, 0, next_neighbor_offset - 1);
	            
	      assert (ret == true);
	            
	      // reset counter
	      std::fill(pack_count.begin(), pack_count.begin() + next_neighbor_offset, 0);
	    }
	    
	  bool succeeded = do_join(query_id);
	    
	  next_neighbor_offset = 0;
	  asking_next = false;
	  if (succeeded)
	    {
	      ReturnedBeliefState* rbs = joined_results.front();
	      joined_results.pop_front();
	      return rbs;
	    }
	}
      else
	{
	  next_neighbor_offset++;
	  asking_next = true;
	}	
    }
}




bool
StreamingJoiner::ask_neighbor_and_receive(std::size_t noff,
					  std::size_t query_id,
					  std::size_t k1,
					  std::size_t k2)
{
  ask_neighbor(noff, query_id, k1, k2);

  int timeout = 0;
  std::size_t count_models_read = 0;

  NewBeliefStateVecPtr& bsv = input_belief_states[noff]; 
  while (1)
    {
      ReturnedBeliefState* rbs = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::JOIN_IN_MQ, noff, timeout);

      if (rbs)
	{
	  std::size_t query_id = rbs->query_id;
	  std::size_t offset = neighbor_offset_from_qid(query_id);
	  assert (noff == offset);
	  ++count_models_read;
	  bsv->push_back(rbs->belief_state);
	}
      else
	{
	  break;
	}
    }

  // now unregister from JoinerDispatcher
  joiner_dispatcher->unregisterIdOffset(query_id, ctx_offset);

  return (count_models_read != 0);
}




bool
StreamingJoiner::do_join(std::size_t query_id)
{
  const NewBeliefStateIteratorVecPtr beg_it(new NewBeliefStateIteratorVec);
  const NewBeliefStateIteratorVecPtr end_it(new NewBeliefStateIteratorVec);

  for (NewBeliefStatePackage::const_iterator it = input_belief_states.begin();
       it != input_belief_states.end(); ++it)
    {
      const NewBeliefStateVecPtr& bsv = *it;
      
      assert (bsv);
      assert (bsv->begin() != bsv->end());

      beg_it->push_back(bsv->begin());
      end_it->push_back(bsv->end());
    }

  return join(query_id, beg_it, end_it);
}



bool
StreamingJoiner::join(std::size_t query_id,
		      const NewBeliefStateIteratorVecPtr& beg_it, 
		      const NewBeliefStateIteratorVecPtr& end_it)
{
  assert ((input_belief_states.size() == beg_it->size()) && (beg_it->size() == end_it->size()));
  bool ret = false;

  std::size_t n = input_belief_states.size();

  NewBeliefStateIteratorVecPtr run_it(new NewBeliefStateIteratorVec);
  
  for (std::size_t i = 0; i < n; ++i)
    {
      run_it->push_back((*beg_it)[i]);
    }

  // recursion disposal
  int inc = n-1;
  while (inc >= 0)
    {
      if (join(query_id, run_it))
	{
	  ret = true;
	}

      inc = n-1;

      // find the last index whose running iterator incrementable to a non-end()
      while (inc >= 0)
	{
	  NewBeliefStateVec::const_iterator& run_it_ref = (*run_it)[inc];
	  run_it_ref++;
	  if (run_it_ref != (*end_it)[inc])
	    {
	      break;
	    }
	  else
	    {
	      --inc;
	    }
	}

      // reset all running iterator after inc to beg_it
      for (std::size_t i = inc+1; i < n; ++i)
	{
	  (*run_it)[i] = (*beg_it)[i];
	}
    }

  return ret;  
}



// one-shot joining
// return true if joining is successful
bool
StreamingJoiner::join(std::size_t query_id,
		      const NewBeliefStateIteratorVecPtr& run_it)
{
  // We don't need the interface V here
  NewBeliefStateIteratorVec::const_iterator it = run_it->begin();

  NewBeliefState* first_bs = **it;
  NewBeliefState* result = new NewBeliefState(*first_bs);

  BeliefStateOffset* bso = BeliefStateOffset::instance();

  for (++it; it != run_it->end(); ++it)
    {
      NewBeliefState* next_bs = **it;
      if (!combine(*result, *next_bs, bso->getStartingOffsets(), bso->getMasks()))
	{
	  delete result;
	  result = 0;

	  return false;
	}
    }


  // Joining succeeded. Now send this input to SAT solver via joiner_sat_notif
  // be careful that we are blocked here. Use timeout sending instead?
  ReturnedBeliefState* rbs = new ReturnedBeliefState(result, query_id);
  joined_results.push_back(rbs);

  return true;
}




bool
StreamingJoiner::ask_first_packs(std::size_t query_id, std::size_t from_neighbor, std::size_t to_neighbor)
{
  assert (0 <= from_neighbor && from_neighbor <= to_neighbor && to_neighbor < no_neighbors);

  for (std::size_t i = from_neighbor; i <= to_neighbor; ++i)
    {
      std::size_t k1 = 0;
      std::size_t k2 = 0;

      if (pack_size > 0)
	{
	  k1 = 1;
	  k2 = pack_size;
	}

      // in this method, we register to joiner_dispatcher
      ask_neighbor(i, query_id, k1, k2);
    }

  // now wait for the models from neighbors (might be returned in a mess)
  
  // mask which neighbor has returned enough models by bits in an integer
  // We don't expect more than 64 neighbors per context, hence this should be enough.

  // turn on bits from (from_neighbor) to (to_neighbor)
  std::size_t all_neighbors_returned = 0;
  all_neighbors_returned = (std::size_t)1 << (to_neighbor - from_neighbor + 1);
  all_neighbors_returned--;
  all_neighbors_returned <<= from_neighbor;

  std::size_t marking_neighbors = 0;
  std::vector<std::size_t> count_models_read(to_neighbor+1, 0);

  int timeout = 0;
  while (marking_neighbors != all_neighbors_returned)
    {
      ReturnedBeliefState* rbs = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::JOIN_IN_MQ, ctx_offset, timeout);

      NewBeliefState* bs = rbs->belief_state;
      std::size_t qid = rbs->query_id;
      std::size_t noff = neighbor_offset_from_qid(qid);
      
      unset_neighbor_offset(qid);
      assert (qid == query_id);

      if (bs)
	{
	  NewBeliefStateVecPtr& bsv = input_belief_states[noff];
	  ++count_models_read[noff];
	  bsv->push_back(bs);
	}
      else
	{
	  if (count_models_read[noff] == 0)
	    {
	      joiner_dispatcher->unregisterIdOffset(query_id, ctx_offset);
	      // this neighbor is inconsistent
	      return false;
	    }

	  // turn on the corresponding bit at noff
	  marking_neighbors |= (std::size_t)1 << noff;
	}      
    }

  // unregister to JoinerDispatcher
  joiner_dispatcher->unregisterIdOffset(query_id, ctx_offset);

  return true;
}



void
StreamingJoiner::ask_neighbor(std::size_t noff, std::size_t query_id, std::size_t k1, std::size_t k2)
{
  cleanup_input(noff);
  joiner_dispatcher->registerIdOffset(query_id, ctx_offset);
  ForwardMessage* request = new ForwardMessage(query_id, k1, k2);

  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_OUT_MQ, noff, request, timeout);
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
