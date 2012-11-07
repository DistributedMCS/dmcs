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
#include "mcs/Logger.h"
#include "mcs/ForwardMessage.h"
#include "mcs/StreamingJoiner.h"

namespace dmcs {

StreamingJoiner::StreamingJoiner(std::size_t ps,
				 NewNeighborVecPtr n)
  : BaseJoiner(n),
    pack_size(ps),
    offset2index(new NeighborOffset2Index),
    next_neighbor(0),
    first_round(true),
    asking_next(false),
    pack_count(n->size(), 0)
{
  // Initialize the map from neighbor_offset (identifying a neighbor's message queue)
  // to an internal index of a vector (identifying the order we use to control the invocation
  // to neighbors
  std::size_t i = 0;
  for (NewNeighborVec::const_iterator it = n->begin(); it != n->end(); ++it, ++i)
    {
      std::size_t neighbor_offset = (*it)->neighbor_offset;
      offset2index->insert(std::make_pair<std::size_t, std::size_t>(neighbor_offset, i));
    }
}



void
StreamingJoiner::reset()
{
  BaseJoiner::reset();

  first_round = true;
  asking_next = false;
  next_neighbor = 0;
  
  // reset counter
  std::fill(pack_count.begin(), pack_count.end(), 0);
}



ReturnedBeliefState*
StreamingJoiner::trigger_join(std::size_t query_id, 
			      const NewHistory& history,
			      NewConcurrentMessageDispatcherPtr md,
			      NewJoinerDispatcherPtr jd)
{
  if (is_shutdown(query_id))
    {
      reset();
      ReturnedBeliefState* end_rbs = new ReturnedBeliefState(NULL, query_id);
      DBGLOG(DBG, "Return NULL");
      return end_rbs;
    }
  else if (!joined_results.empty())
    {
      ReturnedBeliefState* rbs = joined_results.front();
      joined_results.pop_front();
      DBGLOG(DBG, "Return " << *rbs);
      return rbs;
    }
  else
    {
      return process(query_id, history, md, jd);
    }
}



ReturnedBeliefState*
StreamingJoiner::process(std::size_t query_id, 
			 const NewHistory& history,
			 NewConcurrentMessageDispatcherPtr md,
			 NewJoinerDispatcherPtr jd)
{
  if (first_round)
    {
      return first_join(query_id, history, md, jd);
    }
  else if (pack_size > 0)
    {
      return next_join(query_id, history, md, jd);
    }
  else
    {
      reset();
      ReturnedBeliefState* end_rbs = new ReturnedBeliefState(NULL, query_id);
      return end_rbs;
    }
}



ReturnedBeliefState*
StreamingJoiner::first_join(std::size_t query_id, 
			    const NewHistory& history,
			    NewConcurrentMessageDispatcherPtr md,
			    NewJoinerDispatcherPtr jd)
{
  // Warming up round, set first_round to FALSE
  first_round = false;

  if (!ask_first_packs(query_id, history, 0, neighbors->size()-1, md, jd))
    {
      // A neighbor is inconsistent. Reset and return NULL 
      reset();
      ReturnedBeliefState* end_rbs = new ReturnedBeliefState(NULL, query_id);
      return end_rbs;
    }
  
  bool succeeded = do_join(query_id);
  next_neighbor = 0;
  asking_next = false;

  DBGLOG(DBG, "Joined results");
  for (ReturnedBeliefStateList::const_iterator it = joined_results.begin(); it != joined_results.end(); ++it)
    {
      DBGLOG(DBG, **it);
    }

  if (succeeded)
    {
      ReturnedBeliefState* rbs = joined_results.front();
      joined_results.pop_front();
      return rbs;
    }

  if (pack_size > 0)
    {
      succeeded = next_join(query_id, history, md, jd);
      if (succeeded)
	{
	  ReturnedBeliefState* rbs = joined_results.front();
	  joined_results.pop_front();
	  return rbs;
	}
    }

  ReturnedBeliefState* end_rbs = new ReturnedBeliefState(NULL, query_id);
  return end_rbs;
}



ReturnedBeliefState*
StreamingJoiner::next_join(std::size_t query_id, 
			   const NewHistory& history,
			   NewConcurrentMessageDispatcherPtr md,
			   NewJoinerDispatcherPtr jd)
{
  // now really going to the loop of asking next =============================================
  while (1)
    {
      if (next_neighbor == neighbors->size())
	{
	  // No more models from my neighbors	    
	  reset();

	  ReturnedBeliefState* end_rbs = new ReturnedBeliefState(NULL, query_id);
	  return end_rbs;
	}
      
      std::size_t& pc = pack_count[next_neighbor];
      pc++;
      std::size_t k_one = pc * pack_size + 1;
      std::size_t k_two = (pc+1) * pack_size;
      
      // Ask next at neighbor_offset
      if (ask_neighbor_and_receive(next_neighbor, query_id, history, k_one, k_two, md, jd))
	{
	  if (asking_next)
	    {
	      assert (next_neighbor > 0);
	            
	      // Ask first packs before neighbor_offset
	      bool ret = ask_first_packs(query_id, history, 0, next_neighbor - 1, md, jd);
	            
	      assert (ret == true);
	            
	      // reset counter
	      std::fill(pack_count.begin(), pack_count.begin() + next_neighbor, 0);
	    }
	    
	  bool succeeded = do_join(query_id);
	    
	  next_neighbor = 0;
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
	  next_neighbor++;
	  asking_next = true;
	}	
    }
}




bool
StreamingJoiner::ask_neighbor_and_receive(std::size_t neighbor_index,
					  std::size_t forward_qid,
					  const NewHistory& history,
					  std::size_t k1,
					  std::size_t k2,		  
					  NewConcurrentMessageDispatcherPtr md,
					  NewJoinerDispatcherPtr jd)
{
  DBGLOG(DBG, "StreamingJoiner::ask_neighbor_and_receive: forward_qid = " << forward_qid);
  ask_neighbor(neighbor_index, forward_qid, history, k1, k2, md, jd);

  int timeout = 0;
  std::size_t count_models_read = 0;

  NewBeliefStateVecPtr& bsv = input_belief_states[neighbor_index]; 

  while (1)
    {
      ReturnedBeliefState* rbs = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::JOIN_IN_MQ, joiner_offset, timeout);

      NewBeliefState* bs = rbs->belief_state;
      std::size_t qid = rbs->qid;

      if (bs)
	{
	  std::size_t offset = neighbor_offset_from_qid(qid);
	  std::size_t noff = ((*neighbors)[neighbor_index])->neighbor_offset;
	  assert (noff == offset);
	  ++count_models_read;
	  bsv->push_back(rbs->belief_state);
	}
      else
	{
	  // now unregister from JoinerDispatcher
	  DBGLOG(DBG, "StreamingJoiner::ask_neighbor_and_receive: qid = " << qid << detailprint(qid) << ", joiner_offset = " << joiner_offset);
	  jd->unregisterIdOffset(qid, joiner_offset);
	  break;
	}
    }


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

#if 0
  DBGLOG(DBG, "Join:");
  NewBeliefStateIteratorVec::const_iterator bt = beg_it->begin();
  NewBeliefStateIteratorVec::const_iterator et = end_it->begin();
  for (; bt != beg_it->end(); ++bt, ++et)
    {
      for (NewBeliefStateVec::const_iterator it = *bt; it != *et; ++it)
	{
	  DBGLOG(DBG, **it);
	}
      DBGLOG(std::endl);
    }
#endif // 0

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
StreamingJoiner::ask_first_packs(std::size_t forward_qid, 
				 const NewHistory& history,
				 std::size_t from_neighbor, 
				 std::size_t to_neighbor,
				 NewConcurrentMessageDispatcherPtr md,
				 NewJoinerDispatcherPtr jd)
{
  assert (from_neighbor <= to_neighbor && to_neighbor < neighbors->size());

  DBGLOG(DBG, "StreamingJoiner::ask_first_packs: forward_qid = " << forward_qid);

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
      ask_neighbor(i, forward_qid, history, k1, k2, md, jd);
    }

  // now wait for the models from neighbors (might be returned in a mess)
  
  // mask which neighbor has returned enough models by bits in an integer
  // We don't expect more than 64 neighbors per context, hence this should be enough.

  // turn on bits from (from_neighbor) to (to_neighbor)
  std::size_t all_neighbors_returned = 0;
  all_neighbors_returned = (std::size_t)1 << (to_neighbor - from_neighbor + 1);
  all_neighbors_returned--;
  all_neighbors_returned = all_neighbors_returned << from_neighbor;

  std::size_t marking_neighbors = 0;
  std::vector<std::size_t> count_models_read(to_neighbor+1, 0);

  int timeout = 0;
  while (marking_neighbors != all_neighbors_returned)
    {
      ReturnedBeliefState* rbs = md->receive<ReturnedBeliefState>(NewConcurrentMessageDispatcher::JOIN_IN_MQ, joiner_offset, timeout);

      NewBeliefState* bs = rbs->belief_state;
      std::size_t qid = rbs->qid;
      std::size_t noff = neighbor_offset_from_qid(qid);
      std::size_t neighbor_index = (*offset2index)[noff];

      DBGLOG(DBG, "StreamingJoiner::ask_first_packs: got something from noff = " << noff << ", nidex = " << neighbor_index);

      if (bs)
	{
	  std::size_t stripped_qid = qid;

	  DBGLOG(DBG, "StreamingJoiner::ask_first_packs: bs = " << *bs);
	  // neighbor id and offset are set by the neighbor and JoinerDispatcher, respectively.
	  // We have to unset them to compare to the original forward qid
	  unset_neighbor_id(stripped_qid);
	  unset_neighbor_offset(stripped_qid);
	  unset_joiner_offset(stripped_qid);
	  assert (stripped_qid == forward_qid);

	  NewBeliefStateVecPtr& bsv = input_belief_states[neighbor_index];
	  ++count_models_read[neighbor_index];
	  bsv->push_back(bs);
	}
      else // NULL rbs <--> end of pack from JOIN_IN 
	{
	  // unregister from JoinerDispatcher
	  //unset_neighbor_offset(qid);
	  DBGLOG(DBG, "StreamingJoiner::ask_first_packs: unregister qid=" << qid << " " << detailprint(qid) << ", joiner_offset=" << joiner_offset);
	  jd->unregisterIdOffset(qid, joiner_offset);
	  if (count_models_read[neighbor_index] == 0)
	    {
	      // this neighbor is inconsistent
	      return false;
	    }

	  // turn on the corresponding bit at neighbor_index
	  marking_neighbors |= (std::size_t)1 << neighbor_index;
	}      
    }

  return true;
}



void
StreamingJoiner::ask_neighbor(std::size_t neighbor_index, 
			      std::size_t qid, 
			      const NewHistory& history,
			      std::size_t k1, 
			      std::size_t k2,
			      NewConcurrentMessageDispatcherPtr md,
			      NewJoinerDispatcherPtr jd)
{
  cleanup_input(neighbor_index);
  std::size_t neighbor_id = ((*neighbors)[neighbor_index])->neighbor_id;
  std::size_t noff = ((*neighbors)[neighbor_index])->neighbor_offset;

  DBGLOG(DBG, "StreamingJoiner::ask_neighbor: before setting neigbhor: qid = " << qid << " " << detailprint(qid));
  set_neighbor_id(qid, neighbor_id);
  set_neighbor_offset(qid, noff);
  set_joiner_offset(qid, joiner_offset);

  DBGLOG(DBG, "StreamingJoiner::ask_neighbor: qid = " << qid << " " << detailprint(qid) << ", joiner_offset = " << joiner_offset);
  DBGLOG(DBG, "StreamingJoiner::ask_neighbor: jd = " << jd.get());
  DBGLOG(DBG, "StreamingJoiner::ask_first_packs: register qid=" << qid << " " << detailprint(qid) << ", joiner_offset=" << joiner_offset);
  jd->registerIdOffset(qid, joiner_offset);

  ForwardMessage* request = new ForwardMessage(qid, history, k1, k2);

  DBGLOG(DBG, "StreamingJoiner: sending to noff = " << noff << ". With mess = " << *request);
  int timeout = 0;
  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_OUT_MQ, noff, request, timeout);
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
