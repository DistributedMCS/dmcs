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
 * @file   StreamingDMCS.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  20 10:11:24 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "mcs/BeliefState.h"
#include "dmcs/BeliefCombination.h"
#include "dmcs/CommandType.h"
#include "dmcs/Cache.h"
#include "loopformula/DimacsVisitor.h"

#include "dmcs/StreamingDMCS.h"


#include "parser/ClaspResultGrammar.h"
#include "parser/ClaspResultBuilder.h"
#include "parser/ParserDirector.h"

#include "network/Client.h"
#include "network/MessageQueue.h"

#include <vector>
#include <sstream>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

namespace dmcs {


StreamingDMCS::StreamingDMCS(const ContextPtr& c, const TheoryPtr& t, 
			     const SignatureVecPtr& s, 
			     const QueryPlanPtr& query_plan_,
			     std::size_t buf_count_)
  : BaseDMCS(c, t, s),
    query_plan(query_plan_),
    cacheStats(new CacheStats),
    cache(new Cache(cacheStats)),
    system_size(c->getSystemSize()),
    my_id(c->getContextID()),
    buf_count(buf_count_),
    thread_started(false),
    neighbor_input_threads(new ThreadVec)
{ }



StreamingDMCS::~StreamingDMCS()
{ }



ContextPtr
StreamingDMCS::getContext()
{
  return ctx;
}



TheoryPtr
StreamingDMCS::getTheory()
{
  return theory;
}


QueryPlanPtr
StreamingDMCS::getQueryPlan()
{
  return query_plan;
}



SignatureVecPtr
StreamingDMCS::getGlobalSigs()
{
  return global_sigs;
}



// initialize message queues 
void
StreamingDMCS::init_mqs()
{
  std::stringstream str_my_id;
  str_my_id << my_id;

  std::stringstream str_neighbor_id;
  std::string mq_label;

  // message queues for holding answers from neighbors
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);

#if 0
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      NeighborPtr nb = *it;

      str_neighbor_id.str();
      str_neighbor_id << nb->neighbor_id;

      mq_label = DMCS_IN_MQ "-" + str_my_id.str() + "-" + str_neighbor_id.str();
      
      MQ mq_in(boost::interprocess::create_only, 
	       mq_label.c_str(), buf_count, buf_count * DMCS_MQ_MSG_SIZE);
    }

  // message queue for joining input from neighbors
  mq_label = DMCS_JOIN_IN_MQ "-" + str_my_id.str();
  MQ mq_join(boost::interprocess::create_only,
	     mq_label.c_str(), buf_count, buf_count * DMCS_JOIN_IN_MSG_SIZE);
	     
  // message queue for holding local answers from SAT solver
  mq_label = DMCS_OUT_MQ "-" + str_my_id.str();
  MQ mq_out(boost::interprocess::create_only,
	    mq_label.c_str(), buf_count, buf_count * DMCS_MQ_MSG_SIZE);
#endif // 0
}



// remove all message queues
void
StreamingDMCS::remove_mqs()
{
  std::stringstream str_my_id;
  str_my_id << my_id;

  std::stringstream str_neighbor_id;
  std::string mq_label;

  // message queues for holding answers from neighbors
  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);

#if 0
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it)
    {
      NeighborPtr nb = *it;

      str_neighbor_id.str();
      str_neighbor_id << nb->neighbor_id;

      mq_label = DMCS_IN_MQ "-" + str_my_id.str() + "-" + str_neighbor_id.str();
      
      MQ::remove(mq_label.c_str());
    }

  // message queue for joining input from neighbors
  mq_label = DMCS_JOIN_IN_MQ "-" + str_my_id.str();
  MQ::remove(mq_label.c_str());
	     
  // message queue for holding local answers from SAT solver
  mq_label = DMCS_OUT_MQ "-" + str_my_id.str();
  MQ::remove(mq_label.c_str());
#endif //0
}


/*
bool
StreamingDMCS::handleFirstRequest(const StreamingForwardMessage& mess)
{
  const std::size_t c = mess.getInvoker();

#if defined(DEBUG)
  std::cerr << "context " << c << " is calling context " << my_id << std::endl;
#endif // DEBUG

  const NeighborListPtr& nb = query_plan->getNeighbors(my_id);
  
  if (nb->size() == 0) // this is a leaf context
    {
      // local compute without input
    }
  else // this is an intermediate context
    {
      // send request to neighbors
      //sendFirstRequest(nb);
    }
}
*/



void
StreamingDMCS::start_threads(std::size_t invoker)
{
  BeliefStatePtr localV;
  if (invoker == 0)
    {
      localV = query_plan->getGlobalV();
    }
  else
    {
      std::size_t my_id = ctx->getContextID();
      localV = query_plan->getInterface(invoker, my_id);
    }

  const SignaturePtr& sig = ctx->getSignature();
  const SignaturePtr& gsig = createGuessingSignature(localV, sig);

  mixed_sig = ProxySignatureByLocalPtr(new ProxySignatureByLocal(sig, gsig));

  std::cerr << "StreamingDMCS::start_threads. mixed_sig.size() = " << mixed_sig->size() << std::endl;

  ThreadFactory tf(ctx, theory, mixed_sig);

  tf.createNeighborInputThreads(neighbor_input_threads);
  dmcs_thread   = tf.createJoinThread();
  sat_thread    = tf.createLocalSolveThread();
  output_thread = tf.createOutputThread();

  thread_started = true;
}



bool
StreamingDMCS::start_up(const StreamingForwardMessage& mess)
{
  if (!thread_started)
    {
      std::size_t invoker = mess.getInvoker();
      start_threads(invoker);
    }

  /*
  const NeighborListPtr& nb = query_plan->getNeighbors(my_id);
  if (nb->size() == 0) // this is a leaf context
    {
#ifdef DEBUG
      std::cerr << "StreamingDMCS::start_up. Leaf context. Put an epsilon model into SatInputMQ to start the SAT solver without input" << std::endl;
#endif
      // put an epsilon model into SatInputMQ to start the SAT solver without input

    }
  else // this is an intermediate context
    {
#ifdef DEBUG
      std::cerr << "StreamingDMCS::start_up. Intermediate context. Send requests to neighbors by placing a message in each of the NeighborQueryMQ" << std::endl;
#endif

      // send requests to neighbors by placing a message in each of the NeighborQueryMQ

      }  */
}



void
StreamingDMCS::localCompute(BeliefState* input, BeliefState* conflict)
{
  // remove input clauses from previous localCompute

  // add input and conflict (in the same shape of BeliefState, but have different meanings)
  // input: facts 

  // conflict: clause
}

} // namespace dmcs


// Local Variables:
// mode: C++
// End:
