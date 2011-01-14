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
 * @file   NeighborThread.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Fri Jan  14 17:52:24 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEIGHBOR_IN_THREAD_H
#define NEIGHBOR_IN_THREAD_H

#include "dmcs/ConflictNotification.h"
#include "mcs/HashedBiMap.h"
#include "network/connection.hpp"
#include "network/ConcurrentMessageQueueFactory.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

namespace dmcs {

class NeighborThread
{
public:
  NeighborThread(ConflictNotificationFuturePtr& cnf_,
		 MessagingGatewayBCPtr& mg_,
		 const NeighborPtr& nb_,
		 const HashedBiMapPtr& c2o_,
		 const std::size_t invoker_,
		 const std::size_t pack_size_)
    : cnf(cnf_), mg(mg_), nb(nb_), c2o(c2o_),
      invoker(invoker_), pack_size(pack_size_)

  void
  operator()()
  {
    boost::asio::io_service io_service;
    boost::asio::ip::tcp::resolver resolver(io_service);
    boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
    boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint endpoint = *res_it;
    
    // get the offset of the neighbor
    const HashedBiMapByFirst& from_context  = boost::get<Tag::First>(*c2o);
    HashedBiMapByFirst::const_iterator pair = from_context.find(ctx_id);
    const std::size_t offset                = pair->second;

    nip = NeighborInPtr(new NeighborIn(io_service, mg, offset));

    nop = NeighborOutPtr(new NeighborOut(io_service, cnf, offset, invoker, pack_size));

    io_service.run();
  }

private:
  ConflictNotificationFuturePtr cnf;
  MessagingGatewayBCPtr         mg;
  const NeighborPtr             nb;
  const HashedBiMapPtr          c2o;
  const std::size_t             invoker;
  const std::size_t             pack_size;
  NeighborInPtr                 nip;
  NeighborOutPtr                nop;
};

} // namespace dmcs


#endif // NEIGHBOR_THREAD_H

// Local Variables:
// mode: C++
// End:
