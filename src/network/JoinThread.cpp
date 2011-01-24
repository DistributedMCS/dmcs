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
 * @file   JoinThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "dmcs/BeliefCombination.h"
#include "dmcs/ConflictNotification.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/JoinThread.h"

#include "dmcs/Log.h"

namespace dmcs {

JoinThread::JoinThread(std::size_t no_nbs_,
		       std::size_t ss,
		       //const HashedBiMapPtr& c2o_,
		       MessagingGatewayBCPtr& mg_,
		       ConcurrentMessageQueueVecPtr& jnn)
  : no_nbs(no_nbs_),
    system_size(ss),
    //c2o(c2o_),
    mg(mg_),
    joiner_neighbors_notif(jnn)
{ }


bool
JoinThread::import_belief_states(std::size_t noff, std::size_t peq_cnt, 
				 PartialBeliefStatePackagePtr& partial_eqs, 
				 bm::bvector<>& in_mask,
				 bm::bvector<>& pack_full,
				 PartialBeliefStateIteratorVecPtr& beg_it, 
				 PartialBeliefStateIteratorVecPtr& mid_it,
				 ImportStates import_state)
{
  // end of models 
  bool eom = false; 
  const std::size_t offset = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + noff;

  // read BeliefState* from NEIGHBOR_MQ
  PartialBeliefStateVecPtr& bsv = (*partial_eqs)[noff];
  std::size_t mark_prev_end = 0;

  if (import_state == FILLING_UP)
    {
      mark_prev_end = bsv->size();
      /*      mid_it_ref = --bsv->end();
      DMCS_LOG_DEBUG("Begin FILLING UP mode.");
      DMCS_LOG_DEBUG("mid_it_ref == " << **mid_it_ref);*/
    }

  for (std::size_t i = 0; i < peq_cnt; ++i)
    {
      std::size_t prio = 0;
      int timeout = 0;

      //DMCS_LOG_TRACE("Read up to " << peq_cnt << " belief states");

      PartialBeliefState* bs = mg->recvModel(offset, prio, timeout);
      if (bs == 0)
	{
	  // End of Package. Otherwise, the neighbor sent me some crap.
	  assert (i == peq_cnt - 1);

	  DMCS_LOG_TRACE("Reached a NULL model. set bit noff = " << noff);
	 
	  // mark that this neighbor reached its pack_size. 
	  pack_full.set(noff);

	  // NULL models are not used for joining
	  break;
	}

      // normal case, just got a belief state
      //DMCS_LOG_TRACE("Push 1 belief state into bsv");
      bsv->push_back(bs); 
    }
  
  if (import_state != FILLING_UP)
    {
      (*beg_it)[noff] = bsv->begin();
    }
  else
    {
      PartialBeliefStateVec::const_iterator& mid_it_ref = (*mid_it)[noff];
      mid_it_ref = bsv->begin();
      std::advance(mid_it_ref, mark_prev_end);

      /*      DMCS_LOG_DEBUG("Before end FILLING UP mode.");
      DMCS_LOG_DEBUG("mid_it_ref == " << **mid_it_ref);
      ++mid_it_ref;*/
    }

  // turn on the bit that is respective to this context
  in_mask.set(noff);
  //  DMCS_LOG_TRACE("DONE");  

  return eom; 
}



// one-shot joining
std::size_t 
JoinThread::join(const PartialBeliefStateIteratorVecPtr& run_it)
{
  /*  DMCS_LOG_DEBUG(" Join guided by run_it:");
  for (PartialBeliefStateIteratorVec::const_iterator it = run_it->begin();
       it != run_it->end(); ++it)
    {
      DMCS_LOG_DEBUG(" " << ***it);
      }*/

  // We don't need the interface V here
  PartialBeliefStateIteratorVec::const_iterator it = run_it->begin();
  //DMCS_LOG_DEBUG(" Get first belief state");
  PartialBeliefState* first_bs = **it;

  // DMCS_LOG_TRACCE(" Initialize result");
  PartialBeliefState* result = new PartialBeliefState(*first_bs);

  //DMCS_LOG_DEBUG(" Iteratively join with next belief states");
  ++it;
  for (; it != run_it->end(); ++it)
    {
      PartialBeliefState* next_bs = **it;
      if (!combine(*result, *next_bs))
	{
	  //	  DMCS_LOG_DEBUG("INCONSISTENT!\n");

	  return 0;
	}
    }

  DMCS_LOG_DEBUG(" RESULT = " << *result << "\n");

  // Joining succeeded. Now send this input to SAT solver via JOIN_OUT_MQ
  // be careful that we are blocked here. Use timeout sending instead?
  mg->sendModel(result, 0, ConcurrentMessageQueueFactory::JOIN_OUT_MQ, 0);

  return 1;
}



// join in which the range is determined by beg_it to end_it
void
JoinThread::join(const PartialBeliefStatePackagePtr& partial_eqs, 
		 const PartialBeliefStateIteratorVecPtr& beg_it, 
		 const PartialBeliefStateIteratorVecPtr& end_it)
{
  // initialization
  assert ((partial_eqs->size() == beg_it->size()) && (beg_it->size() == end_it->size()));

  /*  DMCS_LOG_DEBUG(" Going to join the following");
  PartialBeliefStateIteratorVec::const_iterator b = beg_it->begin();
  PartialBeliefStateIteratorVec::const_iterator e = end_it->begin();
  for (; b != beg_it->end(); ++b, ++e)
    {
      for (PartialBeliefStateVec::const_iterator it = *b; it != *e; ++it)
	{
	  DMCS_LOG_DEBUG(" " << **it);
	}
      DMCS_LOG_DEBUG(" ");
      }*/

  std::size_t n = partial_eqs->size();

  PartialBeliefStateIteratorVecPtr run_it(new PartialBeliefStateIteratorVec);
  
  for (std::size_t i = 0; i < n; ++i)
    {
      run_it->push_back((*beg_it)[i]);
    }

  // recursion disposal
  int inc = n-1;
  while (inc >= 0)
    {
      join(run_it);
      inc = n-1;

      // find the greates index whose running iterator incrementable to a non-end()
      while (inc >= 0)
	{
	  PartialBeliefStateVec::const_iterator& run_it_ref = (*run_it)[inc];
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
      
      //DMCS_LOG_DEBUG("inc = " << inc);

      // reset all running iterator after inc to beg_it

      for (std::size_t i = inc+1; i < n; ++i)
	{
	  //DMCS_LOG_DEBUG("reset run_it. i = " << i);
	  (*run_it)[i] = (*beg_it)[i];
	  //DMCS_LOG_DEBUG("reset run_it. i = " << i << ". DONE");
	}
    }
  //DMCS_LOG_DEBUG("DONE");
}



void
JoinThread::ask_for_next(PartialBeliefStatePackagePtr& partial_eqs, std::size_t next)
{
  // empty our local storage for the sake of non-exponential space
  PartialBeliefStateVecPtr& bsv = (*partial_eqs)[next];
  bsv->clear();

  // for now, let's put empty conflict and empty ass to notify the neighbors
  ConflictVecPtr empty_conflicts(new ConflictVec);
  PartialBeliefState* empty_ass  = new PartialBeliefState(system_size, PartialBeliefSet());
  ConflictNotification* cn       = new ConflictNotification(0, empty_conflicts, empty_ass);

  ConcurrentMessageQueuePtr& cmq = (*joiner_neighbors_notif)[next];
  
  DMCS_LOG_TRACE(" Will push: conflict = " << *(cn->conflicts)
		 <<", partial_ass = " << *(cn->partial_ass));
  
  ConflictNotification* ow_neighbor = (ConflictNotification*) overwrite_send(cmq, &cn, sizeof(cn), 0);
  
  if (ow_neighbor)
    {
      delete ow_neighbor;
      ow_neighbor = 0;
    }
}

void
JoinThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  ImportStates import_state = START_UP;

  bool stop         = false;
  bool asking_next  = false;

  std::size_t next_neighbor_offset = 0;

  bm::bvector<> in_mask;
  bm::bvector<> pack_full;

  // set up a package of empty BeliefStates
  PartialBeliefStatePackagePtr partial_eqs(new PartialBeliefStatePackage);
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      PartialBeliefStateVecPtr bsv(new PartialBeliefStateVec);
      partial_eqs->push_back(bsv);
    }

  PartialBeliefStateIteratorVecPtr beg_it(new PartialBeliefStateIteratorVec(no_nbs));
  PartialBeliefStateIteratorVecPtr mid_it(new PartialBeliefStateIteratorVec(no_nbs));
  //DMCS_LOG_TRACE("no_nbs = " << no_nbs);

  while (!stop)
    {
      // look at JOIN_IN_MQ for notification of new models arrival
      std::size_t prio = 0;
      int timeout = 0;
      // notification from neighbor thread

      MessagingGatewayBC::JoinIn nn = 
	mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio, timeout);

      DMCS_LOG_TRACE("Received: " << nn.ctx_offset << ", " << nn.peq_cnt);

      if (nn.peq_cnt == 0)
	{ // This neighbor is either out of models of UNSAT

	  assert (import_state != FILLING_UP);

	  if (import_state == START_UP)
	    { // UNSAT
	      
	      // The current C, S to the neighbors is bad. I need to
	      // inform the SAT solver about this by sending him a
	      // NULL model.

	      // Also tell the neighbors (via NeighborOut) to stop
	      // returning models.
	    }
	  else
	    { // out of models, import == GETTING_NEXT

	      // We need to ask the next neighbor (now ordered by the
	      // offset) to give the next package of models.

	      next_neighbor_offset = nn.ctx_offset + 1;
	      //DMCS_LOG_TRACE(" Increase next_neighbor_offset. Now is: " << next_neighbor_offset);
	      if (next_neighbor_offset == no_nbs)
		{
		  stop = true;
		}
	      else
		{
		  asking_next = true;
		  //DMCS_LOG_TRACE(" Going to ask for next with next_neighbor_offset == " << next_neighbor_offset);
		  ask_for_next(partial_eqs, next_neighbor_offset);
		}
	    }
	} // (nn.peq_cnt == 0)
      else
	{ // (nn.peq_cnt != 0)
	  // first we import the belief states received
	  import_belief_states(nn.ctx_offset, nn.peq_cnt, partial_eqs, 
			       in_mask, pack_full,
			       beg_it, mid_it, import_state);

	  DMCS_LOG_TRACE("PartialBeliefState package received: \n" << *partial_eqs);

	  // then if this was a "NEXT" request to this neighbor, we
	  // have to restart the neighbors before him
	  if (asking_next)
	    {
	      DMCS_LOG_TRACE(" Realized that we are in GETTING_NEXT mode.");

	      assert (import_state != START_UP);
	      assert (nn.ctx_offset == next_neighbor_offset);

	      asking_next  = false;
	      import_state = START_UP;

	      for (std::size_t i = 0; i < next_neighbor_offset; ++i)
		{
		  // actually it's asking for a new round of models 
		  ask_for_next(partial_eqs, i);

		  // and reset the in bit mask wrt this neighbor
		  in_mask.set(i, false);

		  // further possible improvement: if neighbor at
		  // offset i has in total less than pack_size models,
		  // then we just keep his models as asking for a new
		  // round will return the same thing.
		}
	    }

	  // At this point, we can check whether joining is possible
	  if (in_mask.count_range(0, no_nbs+1) == no_nbs)
	    {
	      if (import_state == START_UP || import_state == GETTING_NEXT)
		{
		  import_state = FILLING_UP;

		  DMCS_LOG_TRACE("First time joining");

		  for (std::size_t i = 0; i < no_nbs; ++i)
		    {
		      PartialBeliefStateVecPtr& bsv = (*partial_eqs)[i];
		      (*mid_it)[i] = bsv->end();
		    }
		  join(partial_eqs, beg_it, mid_it);
		}
	      else
		{
		  DMCS_LOG_TRACE(" NOT a first time joining ==> do it selectively");
		  for (std::size_t i = 0; i < no_nbs; ++i)
		    {
		      PartialBeliefStateVecPtr& bsv = (*partial_eqs)[i];
		      PartialBeliefStateVec::const_iterator& mid_it_ref = (*mid_it)[i];
		      if (mid_it_ref != bsv->end())
			{
			  // This neighbor has some new models.
			  DMCS_LOG_DEBUG("Neighbor at offset " << i << " has some new models.");
			  PartialBeliefStateVec::const_iterator& beg_it_ref = (*beg_it)[i];
			  
			  // This is the range of new models that we want to join.

			  DMCS_LOG_DEBUG("mid_it = " << **mid_it_ref);

			  beg_it_ref = mid_it_ref;
			  mid_it_ref = bsv->end();
			  
			  join(partial_eqs, beg_it, mid_it);
			  
			  // Restart begin position, because the new models are now all in.
			  beg_it_ref = bsv->begin();
			}
		    }
		}

	      DMCS_LOG_TRACE(" pack_full.count_range == " << pack_full.count_range(0, no_nbs+1));
	      if (pack_full.count_range(0, no_nbs+1) == no_nbs)
		{
		  DMCS_LOG_TRACE(" Pack full, go to get next models");
		  import_state = GETTING_NEXT;
		  asking_next  = true;
		  
		  // always ask for next models from neighbor with offset 0
		  next_neighbor_offset = 0;
		  ask_for_next(partial_eqs, 0);
		}
	    } // (in_mask.count_range(0, no_nbs+1) == no_nbs)

	} // (nn.peq_cnt != 0)

    } // while (!stop)

  DMCS_LOG_TRACE(" DONE. Send a NULL model to JOIN_OUT_MQ");
  mg->sendModel(0, 0, ConcurrentMessageQueueFactory::JOIN_OUT_MQ, 0);
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
