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
 * @file   NeighborThreadFactory.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Jan  11 8:38:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEIGHBOR_INPUT_THREAD_H
#define NEIGHBOR_INPUT_THREAD_H

#include "dmcs/Context.h"
#include "network/BaseClient.h"
#include "network/connection.hpp"
#include "network/ConcurrentMessageQueueFactory.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

class NeighborInputThread
{
public:
  NeighborInputThread(const NeighborPtr& nb_, 
		      std::size_t ctx_id_, 
		      std::size_t pack_size_,
		      std::size_t index_,
		      std::size_t system_size_,
		      boost::shared_ptr<MessagingGateway<BeliefState, Conflict> >& mg_);

  void
  operator()();

private:
  const NeighborPtr nb;
  std::size_t ctx_id;
  std::size_t pack_size;
  std::size_t index;
  std::size_t system_size;
  BaseClient* client;
  boost::shared_ptr<MessagingGateway<BeliefState, Conflict> > mg;
};

} // namespace dmcs


#endif // NEIGHBOR_INPUT_THREAD_H

// Local Variables:
// mode: C++
// End:
