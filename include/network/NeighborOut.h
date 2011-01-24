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

#include "dmcs/ConflictNotification.h"
#include "dmcs/Log.h"
#include "dmcs/StreamingForwardMessage.h"
#include "network/BaseStreamer.h"
#include "solver/Conflict.h"

#include <boost/shared_ptr.hpp>

namespace dmcs {

/**
 * @brief
 */
class NeighborOut : BaseStreamer
{
public:
  NeighborOut(connection_ptr& conn_, 
	      ConcurrentMessageQueuePtr& rnn,
	      std::size_t invoker_, 
	      std::size_t pack_size_)
    : BaseStreamer(conn_),
      router_neighbor_notif(rnn), 
      invoker(invoker_), 
      pack_size(pack_size_)
  {
    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
    stream(boost::system::error_code());
  }

  void 
  stream(const boost::system::error_code& e)
  {
    if (!e)
      {
	// wait for a notification from the Router
	ConflictNotification* cn;
	void *ptr         = static_cast<void*>(&cn);
	unsigned int p    = 0;
	std::size_t recvd = 0;

	DMCS_LOG_TRACE("Listen to router...");
	router_neighbor_notif->receive(ptr, sizeof(cn), recvd, p);

	if (ptr && cn)
	  {
	    ConflictVecPtr conflicts        = cn->conflicts;
	    PartialBeliefState* partial_ass = cn->partial_ass;

	    DMCS_LOG_TRACE("Got from Router: conflict = " << *conflicts << "*partial_ass = " << *partial_ass);

	    // write to network
	    std::string header = HEADER_REQ_STM_DMCS;
	    conn->async_write(header, boost::bind(&NeighborOut::write_message, this,
						  boost::asio::placeholders::error,
						  conflicts, partial_ass));
	  }
	else
	  {
	    DMCS_LOG_FATAL("Got null message: " << ptr << " " << cn);
	    assert(ptr != 0 && cn != 0);
	  }
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }

  

  void
  write_message(const boost::system::error_code& e,
		ConflictVecPtr conflicts, 
		PartialBeliefState* partial_ass)
  {
    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

    if (!e)
      {
	StreamingForwardMessage mess(invoker, pack_size, conflicts, partial_ass);
	conn->async_write(mess, boost::bind(&NeighborOut::clean_up, this,  
					    boost::asio::placeholders::error, conflicts, partial_ass));
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }



  void
  clean_up(const boost::system::error_code& e,
	   ConflictVecPtr conflicts, 
	   PartialBeliefState* partial_ass)
  {
    if (!e)
      {
	// We don't delete partial_ass here. It should be used in other places.

	for (ConflictVec::const_iterator it = conflicts->begin(); it != conflicts->end(); ++it)
	  {
	    Conflict* c = *it;
	    delete c;
	  }

	stream(boost::system::error_code());
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }


private:
  ConcurrentMessageQueuePtr router_neighbor_notif;
  std::size_t               invoker;
  std::size_t               pack_size;
};

typedef boost::shared_ptr<NeighborOut> NeighborOutPtr;

} // namespace dmcs

#endif // NEIGHBOR_IN_H

// Local Variables:
// mode: C++
// End:
