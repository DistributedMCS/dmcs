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
		       const HashedBiMapPtr& c2o_,
		       MessagingGatewayBCPtr& mg_,
		       ConcurrentMessageQueueVecPtr& jnn)
  : no_nbs(no_nbs_),
    system_size(ss),
    c2o(c2o_),
    mg(mg_),
    joiner_neighbors_notif(jnn)
{ }


bool
JoinThread::import_belief_states(std::size_t noff, std::size_t peq_cnt, 
				 BeliefStatePackagePtr& partial_eqs, 
				 bm::bvector<>& in_mask,
				 bm::bvector<>& pack_full,
				 BeliefStateIteratorVecPtr& beg_it, 
				 BeliefStateIteratorVecPtr& mid_it,
				 bool first_import)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " noff = " << noff);

  // end of models 
  bool eom = false; 
  const std::size_t offset = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + noff;

  // read BeliefState* from NEIGHBOR_MQ
  BeliefStateVecPtr& bsv = (*partial_eqs)[noff];

  BeliefStateVec::const_iterator& mid_it_ref = (*mid_it)[noff];

  if (!first_import)
    {
      mid_it_ref = --bsv->end();
    }

  for (std::size_t i = 0; i < peq_cnt; ++i)
    {
      std::size_t prio = 0;
      int timeout = 0;

      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Read up to " << peq_cnt << " belief states");
      BeliefState* bs = mg->recvModel(offset, prio, timeout);
      if (bs == 0)
	{
	  assert (i >= peq_cnt - 2);

	  if (i == peq_cnt - 2)
	    {
	      // This is the sign of eom
	      BeliefState* bs1 = mg->recvModel(offset, prio, timeout);

	      // two NULL models mean end of models
	      assert (bs1 == 0);
	      eom = true;
	    }

	  // Otw, i == peq_cnt-1
	  // This is the sign of end of a package of models

	  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Reached a NULL model. set bit noff = " << noff);
	 
	  // mark that this neighbor reached its pack_size. 
	  pack_full.set(noff);

	  // NULL models are not used for joining
	  break;
	}

      // normal case, just got a belief state
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Push 1 belief state into bsv");
      bsv->push_back(bs); 
    }
  
  if (first_import)
    {
      (*beg_it)[noff] = bsv->begin();
    }
  else
    {
      ++mid_it_ref;
    }

  // turn on the bit that is respective to this context
  in_mask.set(noff);
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " DONE");  

  return eom; 
}



// one-shot joining
std::size_t 
JoinThread::join(const BeliefStateIteratorVecPtr& run_it)
{
  // We don't need the interface V here
  BeliefStateIteratorVec::const_iterator it = run_it->begin();
  BeliefState* first_bs = **it;

  BeliefState* result = new BeliefState(*first_bs);
  
  ++it;
  for (; it != run_it->end(); ++it)
    {
      BeliefState* next_bs = **it;
      if (!combine(*result, *next_bs))
	{
	  return 0;
	}
    }

  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " result = " << *result);

  // Joining succeeded. Now send this input to SAT solver via JOIN_OUT_MQ
  // be careful that we are blocked here. Use timeout sending instead?
  mg->sendModel(result, 0, ConcurrentMessageQueueFactory::JOIN_OUT_MQ, 0);

  return 1;
}



// join in which the range is determined by beg_it to end_it
void
JoinThread::join(const BeliefStatePackagePtr& partial_eqs, 
		 const BeliefStateIteratorVecPtr& beg_it, 
		 const BeliefStateIteratorVecPtr& end_it)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  // initialization
  //assert ((partial_eqs->size() == beg_it->size()) && (beg_it->size() == end_it->size()));
  int n = partial_eqs->size();
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "n = " << n);
  BeliefStateIteratorVecPtr run_it(new BeliefStateIteratorVec);
  
  for (std::size_t i = 0; i < n; ++i)
    {
      run_it->push_back((*beg_it)[i]);
    }

  // recursion disposal
  int inc = n-1;
  while (inc >= 0)
    {
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "inc = " << inc);
      join(run_it);
      inc = n-1;

      // find the greates index whose running iterator incrementable to a non-end()
      while (inc >= 0)
	{
	  BeliefStateVec::const_iterator& run_it_ref = (*run_it)[inc];
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
      
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "inc = " << inc);
      // reset all running iterator after inc to beg_it
      for (int i = inc+1; i < n; ++i)
	{
	  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "reset run_it. i = " << i);
	  (*run_it)[i] = (*beg_it)[i];
	  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "reset run_it. i = " << i << ". DONE");
	}
    }
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " DONE");
}


// This can be a helper function
// return -1 when all neighbors are asked and run out of models
std::size_t
JoinThread::next_to_ask(std::vector<bool>& eom)
{
  // find the first index such that eom[index] = false
  std::size_t index = 0;
  for ( ; index < eom.size(); ++index)
    {
      if (!eom[index])
	{
	  break;
	}
    }

  return index;
}



void
JoinThread::ask_for_next(std::size_t next)
{
  // for now, let's put empty conflict and empty ass to notify the neighbors
  Conflict* empty_conflict       = new Conflict(system_size, BeliefSet());
  BeliefState* empty_ass         = new BeliefState(system_size, BeliefSet());
  ConflictNotification* cn = new ConflictNotification(0, empty_conflict, empty_ass);

  for (std::size_t i = 0; i < next; ++i)
    {
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " First push to offset " << i);
      ConcurrentMessageQueuePtr& cmq = (*joiner_neighbors_notif)[i];
      
      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Will push: conflict = (" << cn->conflict << ") " << *(cn->conflict)
		     <<", partial_ass = (" << cn->partial_ass << ") " << *(cn->partial_ass));
      
      ConflictNotification* ow_neighbor = (ConflictNotification*) overwrite_send(cmq, &cn, sizeof(cn), 0);
      
      if (ow_neighbor)
	{
	  delete ow_neighbor;
	  ow_neighbor = 0;
	}
    }
}

void
JoinThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  // offset of the neighbor that we want the next package of models
  std::vector<bool> eom(no_nbs, false);

  bool stop = false;
  bool first_import = true;
  bm::bvector<> in_mask;
  bm::bvector<> pack_full;
  bm::bvector<> next_mask;

  next_mask.resize(no_nbs+1);
  next_mask.set();           // set all bits to 1

  // set up package of empty BeliefStates
  BeliefStatePackagePtr partial_eqs(new BeliefStatePackage);
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      BeliefStateVecPtr bsv(new BeliefStateVec);
      partial_eqs->push_back(bsv);
    }

  BeliefStateIteratorVecPtr beg_it(new BeliefStateIteratorVec(no_nbs));
  BeliefStateIteratorVecPtr mid_it(new BeliefStateIteratorVec(no_nbs));
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "no_nbs = " << no_nbs);

  while (!stop)
    {
      // look at JOIN_IN_MQ for notification of new models arrival
      std::size_t prio = 0;
      int timeout = 0;
      // notification from neighbor thread

      MessagingGateway<BeliefState, Conflict>::JoinIn nn = 
	mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio, timeout);

      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Received: " << nn.ctx_id << ", " << nn.peq_cnt);

      eom[nn.ctx_id] = import_belief_states(nn.ctx_id, nn.peq_cnt, partial_eqs, 
					 in_mask, pack_full,
					 beg_it, mid_it, first_import);

      // all neighbors have returned some models (not necessarily up to pack_size)
      if (in_mask.count_range(0, no_nbs+1) == no_nbs)
	{
	  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "Time to join");

	  // time to join
	  if (first_import)
	    {
	      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "First time joining");
	      // assign first time join
	      first_import = false;
	      for (std::size_t i = 0; i < no_nbs; ++i)
		{
		  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "i = " << i);
		  BeliefStateVecPtr& bsv = (*partial_eqs)[i];
		  (*mid_it)[i] = bsv->end();
		}
	      join(partial_eqs, beg_it, mid_it);
	    }
	  else
	    {
	      DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << "NOT a first time joining ==> do it selectively");
	      // not the first time, so we just join selectively
	      for (std::size_t i = 0; i < no_nbs; ++i)
		{
		  BeliefStateVecPtr& bsv = (*partial_eqs)[i];
		  BeliefStateVec::const_iterator& mid_it_ref = (*mid_it)[i];
		  if (mid_it_ref != bsv->end())
		    {
		      // This neighbor has some new models.
		      BeliefStateVec::const_iterator& beg_it_ref = (*beg_it)[i];

		      // This is the range of new models that we want to join.
		      beg_it_ref = mid_it_ref;
		      mid_it_ref = bsv->end();

		      join(partial_eqs, beg_it, mid_it);

		      // Restart begin position, because the new models are now all in.
		      beg_it_ref = bsv->begin();
		    }
		}
	    }
	  
	  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " Check for stop. count = " << pack_full.count_range(0, no_nbs+1) << ", no_nbs = " << no_nbs);

	  if (pack_full.count_range(0, no_nbs+1) == no_nbs)
	    {
	      // Now we need to pick a neighbor to ask for next models
	      std::size_t next = next_to_ask(eom);
	      if (next == eom.size())
		{
		  // Really, we have nothing else to expect!
		  stop = true;
		}
	      else
		{
		  for (std::size_t i = 0; i <= next; ++i)
		    {
		      ask_for_next(i);
		      eom[i] = false;
		    }
		}
	    }
	}

      ///@todo: determine the condition for stop = true;
    }
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__ << " DONE");
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
