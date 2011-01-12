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
 * @file   ThreadFactory.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 9:06:24 2011
 * 
 * @brief  
 * 
 * 
 */

#include "network/JoinThread.h"

#include "dmcs/Log.h"

namespace dmcs {

JoinThread::JoinThread(std::size_t no_nbs_,
		       const HashedBiMapPtr& c2o_,
		       boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_)
  : no_nbs(no_nbs_),
    c2o(c2o_),
    mg(mg_)
{ }


void
JoinThread::import_belief_states(std::size_t ctx_id, std::size_t peq_cnt, 
				 BeliefStatePackagePtr partial_eqs, 
				 bm::bvector<>& mask,
				 BeliefStateIteratorVecPtr beg_it, 
				 BeliefStateIteratorVecPtr mid_it)
{
  const HashedBiMapByFirst& from_context = boost::get<Tag::First>(*c2o);
  HashedBiMapByFirst::const_iterator pair = from_context.find(ctx_id);
  std::size_t index = pair->second;
  const std::size_t off = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + 2*index;

  // read BeliefState* from NEIGHBOR_MQ
  BeliefStateVecPtr bsv = (*partial_eqs)[index];
  bool first_import = false;

  if (bsv->size() == 0)
    {
      first_import = true;
    }

  if (!first_import)
    {
      (*mid_it)[index] = --bsv->end();
    }

  for (std::size_t i = 0; i < peq_cnt; ++i)
    {
      std::size_t prio = 0;
      std::size_t timeout = 0;
      BeliefState* bs = mg->recvModel(off, prio, timeout);
      bsv->push_back(bs); 
    }
  
  if (first_import)
    {
      (*beg_it)[index] = bsv->begin();
      (*mid_it)[index] = bsv->begin();
    }
  else
    {
      (*mid_it)[index]++;
    }

  // turn on the bit that is respective to this context
  mask.set(index);
}



void
JoinThread::join(BeliefStatePackagePtr partial_eqs, 
		 BeliefStateIteratorVecPtr beg_it, 
		 BeliefStateIteratorVecPtr mid_it)
{

  // mark the completion of this join
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      BeliefStateVecPtr bsv = (*partial_eqs)[i];

      (*mid_it)[i] = --(bsv->end());
    }
}


void
JoinThread::operator()()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  bool stop = false;
  bm::bvector<> mask;

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
      std::size_t timeout = 0;
      // notification from neighbor thread
      MessagingGateway<BeliefState, Conflict>::JoinIn nn = 
	mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio, timeout);

      import_belief_states(nn.ctx_id, nn.peq_cnt, partial_eqs, mask, beg_it, mid_it);

      // all neighbors have returned some models (not necessarily pack_size)
      if (mask.count_range(0, no_nbs) == no_nbs)
	{
	  // time to join
	}

      ///@todo: determine the condition for stop = true;
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
