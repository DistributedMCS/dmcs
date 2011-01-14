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
 * @file   NeighborOut.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Jan  13 23:12:21 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef NEIGHBOR_OUT_H
#define NEIGHBOR_OUT_H

#include "dmcs/Log.h"
#include "mcs/Conflict.h"
#include "network/BaseStreamer.h"

namespace dmcs {

/**
 * @brief
 */
class NeighborOut : BaseStreamer
{
public:
  NeighborOut(connection_ptr conn_, MessagingGatewayBC mg_,
	      std::size_t noff_,std::size_t invoker_, 
	      std::size_t pack_size)
    : BaseStreamer(conn_, mg_, noff_),
      invoker(invoker_), pack_size(pack_size_)
  { }

  void 
  stream(const boost::system::error_code& e)
  {
    if (!e)
      {
	// read from NEIGHBOR_OUT_MQ + noff
	std::size_t prio = 0;
	std::size_t timeout = 0;
	Conflict* conflict = mg->recvConflict(ConcurrentMessageQueueFactory::NEIGHBOR_OUT_MQ + noff,
					      prio, timeout);

	StreamingForwardMessage mess(invoker, pack_size, conflict);

	// write to network
	conn->async_write(mess, boost::bind(&NeighborOut::stream, this,  
					   boost::asio::placeholders::error));
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }

private:
  std::size_t invoker;
  std::size_t pack_size;
};

} // namespace dmcs

#endif // NEIGHBOR_IN_H

// Local Variables:
// mode: C++
// End:
