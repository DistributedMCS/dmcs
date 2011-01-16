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
#include "network/JoinThread.h"

#include "dmcs/Log.h"

namespace dmcs {

JoinThread::JoinThread(std::size_t no_nbs_,
		       const HashedBiMapPtr& c2o_,
		       MessagingGatewayBCPtr& mg_)
  : no_nbs(no_nbs_),
    c2o(c2o_),
    mg(mg_)
{ }


void
JoinThread::import_belief_states(std::size_t noff, std::size_t peq_cnt, 
				 BeliefStatePackagePtr& partial_eqs, 
				 bm::bvector<>& in_mask,
				 bm::bvector<>& end_mask,
				 BeliefStateIteratorVecPtr& beg_it, 
				 BeliefStateIteratorVecPtr& mid_it,
				 bool first_import)
{
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
      BeliefState* bs = mg->recvModel(offset, prio, timeout);
      if (bs == 0)
	{
	  // It must be the last model to be NULL;
	  // otw, the neighbor sent me some crap
	  assert (i == peq_cnt - 1);
	 
	  // mark that this neighbor reached its pack_size. 
	  end_mask.set(noff);

	  // NULL models are not used for joining
	  break;
	}
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
  // initialization
  assert ((partial_eqs->size() == beg_it->size()) && (beg_it->size() == end_it->size()));
  std::size_t n = partial_eqs->size();
  BeliefStateIteratorVecPtr run_it(new BeliefStateIteratorVec);
  
  for (std::size_t i = 0; i < n; ++i)
    {
      run_it->push_back((*beg_it)[i]);
    }

  // recursion disposal
  std::size_t inc = n-1;
  while (inc > 0)
    {
      join(run_it);
      inc = n-1;

      // find the greates index whose running iterator incrementable to a non-end()
      while (inc > 0)
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
      
      // reset all running iterator after inc to beg_it
      for (std::size_t i = inc+1; i < n; ++i)
	{
	  (*run_it)[i] = (*beg_it)[i];
	}
    }
}


void
JoinThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  bool stop = false;
  bool first_import = true;
  bm::bvector<> in_mask;
  bm::bvector<> end_mask;

  // set up package of empty BeliefStates
  BeliefStatePackagePtr partial_eqs(new BeliefStatePackage);
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      BeliefStateVecPtr bsv(new BeliefStateVec);
      partial_eqs->push_back(bsv);
    }

  BeliefStateIteratorVecPtr beg_it(new BeliefStateIteratorVec(no_nbs));
  BeliefStateIteratorVecPtr mid_it(new BeliefStateIteratorVec(no_nbs));

  while (!stop)
    {
      DMCS_LOG_DEBUG("no_nbs = " << no_nbs);

      // look at JOIN_IN_MQ for notification of new models arrival
      std::size_t prio = 0;
      int timeout = 0;
      // notification from neighbor thread
      MessagingGateway<BeliefState, Conflict>::JoinIn nn = 
	mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio, timeout);

      import_belief_states(nn.ctx_id, nn.peq_cnt, partial_eqs, 
			   in_mask, end_mask,
			   beg_it, mid_it, first_import);

      // all neighbors have returned some models (not necessarily up to pack_size)
      if (in_mask.count_range(0, no_nbs+1) == no_nbs)
	{
	  // time to join
	  if (first_import)
	    {
	      // assign first time join
	      first_import = false;
	      for (std::size_t i = 0; i < no_nbs; ++i)
		{
		  BeliefStateVecPtr& bsv = (*partial_eqs)[i];
		  (*mid_it)[i] = bsv->end();
		}
	      join(partial_eqs, beg_it, mid_it);
	    }
	  else
	    {
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
	  
	  if (end_mask.count_range(0, no_nbs+1) == no_nbs)
	    {
	      // all neighbors reached their pack_size. Need a
	      // strategy to find a neighbor to ask for next models.
	      // for now let stop
	      stop = true;
	    }
	}

      ///@todo: determine the condition for stop = true;
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
