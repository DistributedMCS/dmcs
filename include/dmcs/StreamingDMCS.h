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
 * @file   StreamingDMCS.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Dec  2010 09:48:21 2010
 * 
 * @brief  
 * 
 * 
 */

#ifndef STREAMING_DMCS_H
#define STREAMING_DMCS_H

#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include "dmcs/BaseDMCS.h"
#include "dmcs/StreamingForwardMessage.h"
#include "dmcs/ReturnMessage.h"
#include "dmcs/Cache.h"
#include "dmcs/QueryPlan.h"
#include "network/MessageQueueFactory.h"
#include "network/ThreadFactory.h"
#include "mcs/Theory.h"
#include "solver/Conflict.h"

namespace dmcs {

class StreamingDMCS : public BaseDMCS
{
public:

#if defined(DMCS_STATS_INFO)
  ///@todo: MD: define appropriate type wrt. DMCS_STATS_INFO or not. Keep
  /// in mind that with StreamingDMCS, we don't return BeliefStates
  /// but rather send them to a message queue
#else
  typedef bool dmcs_value_type;
  typedef bool dmcs_return_type;
#endif

  StreamingDMCS(const ContextPtr& c, const TheoryPtr& t, const SignatureVecPtr& s, const QueryPlanPtr& query_plan_, std::size_t buf_count_);

  virtual
  ~StreamingDMCS();

  bool
  start_up(const StreamingForwardMessage& mess);

  bool
  start_up(const StreamingForwardMessage& mess, std::size_t port);

private:
  void
  start_threads(std::size_t invoker, std::size_t pack_size);

private:
  QueryPlanPtr query_plan;
  CacheStatsPtr cacheStats;
  CachePtr cache;

  bool thread_started;
  bool mqs_created;

  ThreadVecPtr neighbor_input_threads;
  boost::thread* dmcs_thread;
  boost::thread* sat_thread;

  ProxySignatureByLocalPtr mixed_sig;

  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;

  std::size_t system_size;
  std::size_t my_id;
  std::size_t buf_count;
};

typedef boost::shared_ptr<StreamingDMCS> StreamingDMCSPtr;


} // namespace dmcs


#endif // STREAMING_DMCS_H

// Local Variables:
// mode: C++
// End:
