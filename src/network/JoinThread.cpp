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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif // HAVE_CONFIG_H

#include <algorithm>

#include <boost/functional/hash.hpp>

#include "dmcs/AskNextNotification.h"
#include "dmcs/BeliefCombination.h"
#include "dmcs/SessionNotification.h"
#include "network/ConcurrentMessageQueueHelper.h"
#include "network/JoinThread.h"

#include "dmcs/Log.h"



namespace dmcs {



JoinThread::JoinThread(std::size_t p,
		       std::size_t cid,
		       std::size_t sid,
		       JoinerDispatcher* jd)
  : port(p),
    ctx_id(cid),
    session_id(sid),
    joiner_dispatcher(jd),
    first_result(true),
    joined_results(new ModelSessionIdList),
    input_queue(new ConcurrentMessageQueue)
{ }



JoinThread::~JoinThread()
{
  DMCS_LOG_TRACE(port << ": Terminating JoinThread.");
}



// one-shot joining
// return true if joining is successful
bool
JoinThread::join(const PartialBeliefStateIteratorVecPtr& run_it)
{
#if 1
  DMCS_LOG_TRACE(port << ": Join guided by run_it:");
  for (PartialBeliefStateIteratorVec::const_iterator it = run_it->begin();
       it != run_it->end(); ++it)
    {
      DMCS_LOG_DEBUG(" " << ***it);
    }
#endif //0

  // We don't need the interface V here
  PartialBeliefStateIteratorVec::const_iterator it = run_it->begin();

  PartialBeliefState* first_bs = **it;
  PartialBeliefState* result = new PartialBeliefState(*first_bs);

  for (++it; it != run_it->end(); ++it)
    {
      PartialBeliefState* next_bs = **it;
      if (!combine(*result, *next_bs))
	{
	  DMCS_LOG_TRACE(port << ": INCONSISTENT!");
	  delete result;
	  result = 0;

	  return false;
	}
    }



  // Joining succeeded. Now send this input to SAT solver via joiner_sat_notif
  // be careful that we are blocked here. Use timeout sending instead?
  if (first_result)
    {
      DMCS_LOG_TRACE(port << ": Final RESULT = " << result << ":" << *result << " ... Sending to joiner_sat_notif");

      struct MessagingGatewayBC::ModelSession ms = {result, path, session_id};
      joiner_sat_notif->send(&ms, sizeof(ms), 0);

      first_result = false;
    }
  else
    {
      DMCS_LOG_TRACE(port << ": Final RESULT = " << result << ":" << *result << " ... push into joined_results");
      ModelSessionId ms(result, path, session_id);
      joined_results->push_back(ms);
    }

  return true;
}



// join in which the range is determined by beg_it to end_it
// return true if there is at least one successful join
bool
JoinThread::join(const PartialBeliefStatePackagePtr& partial_eqs, 
		 const PartialBeliefStateIteratorVecPtr& beg_it, 
		 const PartialBeliefStateIteratorVecPtr& end_it)
{
  // initialization
  assert ((partial_eqs->size() == beg_it->size()) && (beg_it->size() == end_it->size()));
  bool ret = false;

#if 1
  DMCS_LOG_DEBUG(" Going to join the following");
  PartialBeliefStateIteratorVec::const_iterator b = beg_it->begin();
  PartialBeliefStateIteratorVec::const_iterator e = end_it->begin();
  for (; b != beg_it->end(); ++b, ++e)
    {
      for (PartialBeliefStateVec::const_iterator it = *b; it != *e; ++it)
	{
	  DMCS_LOG_DEBUG(" " << *it << ": " << **it);
	}
      DMCS_LOG_DEBUG(" ");
      }
#endif // 0

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
      if (join(run_it))
	{
	  ret = true;
	}

      inc = n-1;

      // find the last index whose running iterator incrementable to a non-end()
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

      // reset all running iterator after inc to beg_it
      for (std::size_t i = inc+1; i < n; ++i)
	{
	  (*run_it)[i] = (*beg_it)[i];
	}
    }

  return ret;
}




void
JoinThread::reset(bool& first_round,
		  bool& asking_next,
		  std::size_t next_neighbor_offset,
		  PartialBeliefStatePackagePtr& partial_eqs,
		  VecSizeTPtr& pack_count)
{
  DMCS_LOG_TRACE("Reset Joiner");

  // reset for the next fresh request
  first_round = true;
  asking_next = false;
  first_result = true;
  next_neighbor_offset = 0;

  for (ModelSessionIdList::const_iterator it = joined_results->begin(); 
       it != joined_results->end(); ++it)
    {
      ModelSessionId ms = *it;
      PartialBeliefState* result = ms.partial_belief_state;
      delete result;
      result = 0;
    }

  joined_results->clear();
  
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      cleanup_partial_belief_states(partial_eqs.get(), i);
    }
  
  std::fill(pack_count->begin(), pack_count->end(), 0);
  DMCS_LOG_TRACE("Reset Joiner. first_round = " << first_round);
}



// remove parameter s
void
JoinThread::operator()(std::size_t nbs,
		       std::size_t s,
		       MessagingGatewayBC* m,
		       ConcurrentMessageQueue* jsn,
		       ConcurrentMessageQueue* sjn,
		       ConcurrentMessageQueueVec* jv)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  mg = m;
  joiner_sat_notif = jsn;
  sat_joiner_notif = sjn;
  joiner_neighbors_notif = jv;
  no_nbs = nbs;
  bool first_round = true;
  bool asking_next = false;
  std::size_t next_neighbor_offset = 0;
  VecSizeTPtr pack_count(new VecSizeT(no_nbs, 0));

  // set up a package of empty BeliefStates
  PartialBeliefStatePackagePtr partial_eqs(new PartialBeliefStatePackage);
  for (std::size_t i = 0; i < no_nbs; ++i)
    {
      PartialBeliefStateVecPtr bsv(new PartialBeliefStateVec);
      partial_eqs->push_back(bsv);
    }

  while (1)
    {
      DMCS_LOG_TRACE("New round in while loop. first_round = " << first_round << ". Wait for SAT TRIGGER");
      // Wait for a trigger from SAT
      std::size_t prio = 0;
      int timeout = 0;
      AskNextNotification* sat_trigger = receive_notification(sat_joiner_notif);

      DMCS_LOG_TRACE("Got a trigger from SAT. AskNextNotification = " << *sat_trigger);

      BaseNotification::NotificationType nt = sat_trigger->type;
      std::size_t path = sat_trigger->path;
      std::size_t session_id = sat_trigger->session_id;
      std::size_t k1 = sat_trigger->k1;
      std::size_t k2 = sat_trigger->k2;

      delete sat_trigger;
      sat_trigger = 0;
      
      if (nt == BaseNotification::NEXT)
	{
	  if (!joined_results->empty())
	    {
	      DMCS_LOG_TRACE("joined_results->size() = " << joined_results->size());
	      DMCS_LOG_TRACE("Still have some joined input left, return now.");
	      ModelSessionId ms = joined_results->front();
	      joined_results->pop_front();
	      PartialBeliefState* result = ms.partial_belief_state;
	      std::size_t pa = ms.path;
	      std::size_t sid = ms.session_id;

	      ///@todo: Fix ModelSession v.s. ModelSessionId duplication
	      struct MessagingGatewayBC::ModelSession ms1 = { result, pa, sid };
	      joiner_sat_notif->send(&ms1, sizeof(ms1), 0);
	    }
	  else
	    {
	      DMCS_LOG_TRACE("joined_results is empty, call process() now. first_round = " << first_round);
	      first_result = true;

	      process(path, session_id, k1, k2, first_round, asking_next, next_neighbor_offset, pack_count, partial_eqs);
	    }
	}
      else
	{
	  assert (nt == BaseNotification::SHUTUP);
	  DMCS_LOG_TRACE("In SHUTUP mode, reset all data members");

	  reset(first_round, asking_next,
		next_neighbor_offset,
		partial_eqs,
		pack_count);

	  DMCS_LOG_TRACE("After reset. first_round = " << first_round);
	}
    }
}



void
JoinThread::cleanup_partial_belief_states(PartialBeliefStatePackage* partial_eqs, std::size_t noff)
{
  DMCS_LOG_TRACE("Clean up PartialBeliefStates at nid = " << noff);
  PartialBeliefStateVecPtr& bsv = (*partial_eqs)[noff];
  DMCS_LOG_TRACE("Number of PartialBeliefStates to clean up = " << bsv->size());
  for (PartialBeliefStateVec::iterator it = bsv->begin(); it != bsv->end(); ++it)
    {
      assert (*it);

      DMCS_LOG_TRACE("Will delete it = " << *it << ": " << **it);

      delete *it;
      *it = 0;
    }
  
  bsv->clear();
}



bool
JoinThread::ask_neighbor(PartialBeliefStatePackage* partial_eqs,
			 std::size_t noff, 
			 std::size_t k1, 
			 std::size_t k2,
			 std::size_t& path,
			 BaseNotification::NotificationType nt)
{
  assert (k1 <= k2);

  DMCS_LOG_TRACE("Send to neighbor: noff = " << noff << ", path = " << path << ", k1 = " << k1 << ", k2 = " << k2);

  // clean up if necessary =====================================================================================
  if (partial_eqs)
    {
      // empty our local storage for the sake of non-exponential space
      cleanup_partial_belief_states(partial_eqs, noff);
    }
  
  // now send the request =======================================================================================
  std::size_t new_path = path;
  boost::hash_combine(path, ctx_id);
  joiner_dispatcher->registerThread(new_path, input_queue.get());
  AskNextNotification* ann = new AskNextNotification(nt, new_path, session_id, k1, k2);

  ConcurrentMessageQueuePtr& cmq = (*joiner_neighbors_notif)[noff];
  
  AskNextNotification* anw_neighbor = (AskNextNotification*) overwrite_send(cmq.get(), &ann, sizeof(ann), 0);
  
  if (anw_neighbor)
    {
      delete anw_neighbor;
      anw_neighbor = 0;
    }

  // and wait for result (as we now allow only one path to be active at a time)
  // change here to allow parallelism
  std::size_t prio = 0;
  int timeout = 0;

  //  MessagingGatewayBC::JoinIn nn = 
  //  mg->recvJoinIn(ConcurrentMessageQueueFactory::JOIN_IN_MQ, prio, timeout);

  std::size_t count_models_read = 0;

  PartialBeliefStateVecPtr& bsv = (*partial_eqs)[noff];
  while (1)
    {
      struct MessagingGatewayBC::ModelSession ms = receive_model(input_queue.get());
      PartialBeliefState* bs = ms.m;
      std::size_t sid = ms.sid;
      if (bs)
	{
	  count_models_read++;
	  DMCS_LOG_TRACE("Got bs = " << bs << ": " << *bs << ". sid = " << sid);

	  if (sid == session_id)
	    {
	      DMCS_LOG_TRACE("Storing belief state " << count_models_read << " from " << noff);
	      bsv->push_back(bs);
	    }
	  else
	    {
	      DMCS_LOG_TRACE("Ignore this belief state because it belongs to an old session");
	      delete ms.m;
	      ms.m = 0;
	    }
	}
      else
	{
	  break;
	}
    }

  // now unregister from JoinerDispatcher
  joiner_dispatcher->unRegisterThread(new_path);

  if (count_models_read == 0)
    {
      return false;
    }

  /*
  MessagingGatewayBC::JoinIn nn = receive_join_in(input_queue.get());
	  
  DMCS_LOG_TRACE(port << ": Received from offset " << nn.ctx_offset << ", " << nn.peq_cnt << " peqs to pick up.");
  
  if (nn.peq_cnt == 0)
    {
      // EOF
      DMCS_LOG_TRACE(port << ": Bailing out.");
      return false;
    }
  else
    {
      // import nn.peq_cnt models ===============================================================================
      PartialBeliefStateVecPtr& bsv = (*partial_eqs)[noff];
      const std::size_t offset = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + noff;

      for (std::size_t i = 0; i < nn.peq_cnt; ++i)
	{
	  std::size_t prio = 0;
	  int timeout = 0;
	  
	  //struct MessagingGatewayBC::ModelSession ms = mg->recvModel(offset, prio, timeout);
	  struct MessagingGatewayBC::ModelSession ms = receive_model(input_queue.get());
	  PartialBeliefState* bs = ms.m;
	  std::size_t sid = ms.sid;

	  assert (bs);

	  DMCS_LOG_TRACE("Got bs = " << bs << ": " << *bs << ". sid = " << sid);

	  if (sid == session_id)
	    {
	      DMCS_LOG_TRACE("Storing belief state " << i << " from " << noff);
	      bsv->push_back(bs);
	    }
	  else
	    {
	      DMCS_LOG_TRACE("Ignore this belief state because it belongs to an old session");
	      delete ms.m;
	      ms.m = 0;
	    }
	}
	}*/

  return true;
}



bool
JoinThread::ask_first_packs(PartialBeliefStatePackage* partial_eqs,
			    std::size_t& path, 
			    std::size_t from_neighbor, 
			    std::size_t to_neighbor)
{
  assert (0 <= from_neighbor && from_neighbor <= to_neighbor && to_neighbor < no_nbs);

  for (std::size_t i = from_neighbor; i <= to_neighbor; ++i)
    {
      std::size_t k1 = 0;
      std::size_t k2 = 0;

      if (pack_size > 0)
	{
	  k1 = 1;
	  k2 = pack_size;
	}

      if (!ask_neighbor(partial_eqs, i, k1, k2, path, BaseNotification::REQUEST))
	{
	  DMCS_LOG_TRACE("A neighbor is inconsistent. Going to clean up everything readd so far.");
	  // clean up
	  for (std::size_t j = from_neighbor; j <= i; ++j)
	    {
	      cleanup_partial_belief_states(partial_eqs, j);
	    }

	  return false;
	}
    }
  return true;
}



// return true if at least one join is successful
bool
JoinThread::do_join(PartialBeliefStatePackagePtr& partial_eqs)
{
  const PartialBeliefStateIteratorVecPtr beg_it(new PartialBeliefStateIteratorVec);
  const PartialBeliefStateIteratorVecPtr end_it(new PartialBeliefStateIteratorVec);

  for (PartialBeliefStatePackage::const_iterator it = partial_eqs->begin();
       it != partial_eqs->end(); ++it)
    {
      const PartialBeliefStateVecPtr& bsv = *it;
      
      assert (bsv);
      assert (bsv->begin() != bsv->end());

      beg_it->push_back(bsv->begin());
      end_it->push_back(bsv->end());
    }

  return join(partial_eqs, beg_it, end_it);
}



void
JoinThread::first_join(std::size_t path, 
		       std::size_t session_id,
		       std::size_t k_one, 
		       std::size_t k_two,
		       bool& first_round,
		       bool& asking_next,
		       std::size_t& next_neighbor_offset,
		       VecSizeTPtr& pack_count,
		       PartialBeliefStatePackagePtr& partial_eqs)
{
  if (k_two == 0)
    {
      pack_size = 0;
    }
  else
    {
      pack_size = k_two - k_one + 1;
    }

  // Warming up round ======================================================================= 
  DMCS_LOG_TRACE("Set first_round to FALSE");
  first_round = false;
  if (!ask_first_packs(partial_eqs.get(), path, 0, no_nbs-1))
    {
      DMCS_LOG_TRACE("A neighbor is inconsistent. Send a NULL model to joiner_sat_notif");
      
      reset(first_round, asking_next,
	    next_neighbor_offset,
	    partial_eqs,
	    pack_count);
      
      DMCS_LOG_TRACE("After reset. first_round = " << first_round);

      struct MessagingGatewayBC::ModelSession ms = { 0, 0, 0 };
      joiner_sat_notif->send(&ms, sizeof(ms), 0);
      
      DMCS_LOG_TRACE("Bailing out...");
      
      return;
    }
  
  bool send_something = do_join(partial_eqs);
  next_neighbor_offset = 0;
  asking_next = false;


  if (!send_something && k_two > 0)
    {
      next_join(path, session_id, k_one, k_two,
		first_round, asking_next, 
		next_neighbor_offset,
		pack_count, partial_eqs);
    }
}



void
JoinThread::next_join(std::size_t path, 
		      std::size_t session_id,
		      std::size_t k_one, 
		      std::size_t k_two,
		      bool& first_round,
		      bool& asking_next,
		      std::size_t& next_neighbor_offset,
		      VecSizeTPtr& pack_count,
		      PartialBeliefStatePackagePtr& partial_eqs)
{
  if (k_two == 0)
    {
      pack_size = 0;
    }
  else
    {
      pack_size = k_two - k_one + 1;
    }

  // now really going to the loop of asking next =============================================
  while (1)
    {
      if (next_neighbor_offset == no_nbs)
	{
	  DMCS_LOG_TRACE("No more models from my neighbors. Send a NULL model to joiner_sat_notif");
	  
	  reset(first_round, asking_next,
		next_neighbor_offset,
		partial_eqs,
		pack_count);
	  
	  DMCS_LOG_TRACE("After reset. first_round = " << first_round);

	  struct MessagingGatewayBC::ModelSession ms = { 0, 0, 0 };
	  joiner_sat_notif->send(&ms, sizeof(ms), 0);

	  return;
	}
      
      std::size_t& pc = (*pack_count)[next_neighbor_offset];
      pc++;
      std::size_t k1 = pc * pack_size + 1;
      std::size_t k2 = (pc+1) * pack_size;
      DMCS_LOG_TRACE("New k1 = " << k1 << ", new k2 = " << k2);
      //DMCS_LOG_TRACE("Pack count: ");
      //std::copy(pack_count->begin(), pack_count->end(), std::ostream_iterator<std::size_t>(std::cerr, " "));
      //std::cerr << std::endl;
      
      DMCS_LOG_TRACE("Ask next at neighbor_offset = " << next_neighbor_offset);
      if (ask_neighbor(partial_eqs.get(), next_neighbor_offset, k1, k2, path, BaseNotification::NEXT))
	{
	  if (asking_next)
	    {
	      assert (next_neighbor_offset > 0);
	      
	      DMCS_LOG_TRACE("Ask first packs before neighbor_offset = " << next_neighbor_offset);
	      // again, ask for first packs from each neighbor
	      bool ret = ask_first_packs(partial_eqs.get(), path, 0, next_neighbor_offset - 1);
	      
	      assert (ret == true);
	      
	      // reset counter
	      std::fill(pack_count->begin(), pack_count->begin() + next_neighbor_offset, 0);
	    }
	  
	  bool send_something = do_join(partial_eqs);
	  
	  next_neighbor_offset = 0;
	  asking_next = false;
	  if (send_something)
	    {
	      break;
	    }
	}
      else
	{
	  next_neighbor_offset++;
	  DMCS_LOG_TRACE("Try next neighbor " << next_neighbor_offset);
	  asking_next = true;
	}
    }
}



void
JoinThread::process(std::size_t path, 
		    std::size_t session_id,
		    std::size_t k_one, 
		    std::size_t k_two,
		    bool& first_round,
		    bool& asking_next,
		    std::size_t& next_neighbor_offset,
		    VecSizeTPtr& pack_count,
		    PartialBeliefStatePackagePtr& partial_eqs)
{
  if (first_round)
    {
      first_join(path, session_id, k_one, k_two,
		 first_round, asking_next, 
		 next_neighbor_offset,
		 pack_count, partial_eqs);
    }
  else if (k_two > 0)
    {
      next_join(path, session_id, k_one, k_two,
		first_round, asking_next, 
		next_neighbor_offset,
		pack_count, partial_eqs);
    }
  else
    {
      struct MessagingGatewayBC::ModelSession ms = { 0, 0, 0 };
      joiner_sat_notif->send(&ms, sizeof(ms), 0);

      reset(first_round, asking_next,
	    next_neighbor_offset,
	    partial_eqs,
	    pack_count);
    }
}



ConcurrentMessageQueue*
JoinThread::getCMQ()
{
  return input_queue.get();
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
