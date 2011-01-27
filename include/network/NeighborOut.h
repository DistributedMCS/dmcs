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
#include "solver/Conflict.h"
#include "network/connection.hpp"

#include <boost/shared_ptr.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>


namespace dmcs {

/**
 * @brief
 */
class NeighborOut
{
public:
  NeighborOut()
  { }


  void
  operator() (connection_ptr conn,
	      ConcurrentMessageQueue* rnn,
	      std::size_t invoker, 
	      std::size_t nid,
	      std::size_t pack_size)
  {
    while (1)
      {
	DMCS_LOG_TRACE("Wait for router notification, nid = " << nid);

	ConflictNotification* cn = wait_router(rnn);

	if (cn->type == ConflictNotification::SHUTDOWN)
	  {
	    DMCS_LOG_TRACE("Neighbor " << nid << " out.");
	    return;
	  }

	ConflictVec* conflicts = cn->conflicts;
	PartialBeliefState* partial_ass = cn->partial_ass;

	DMCS_LOG_TRACE("Got from Router: conflict = " << *conflicts << "*partial_ass = " << *partial_ass);

	// write to network
	const std::string& header = HEADER_REQ_STM_DMCS;
	conn->async_write(header,
			  boost::bind(&NeighborOut::handle_write_message, this,
				      boost::asio::placeholders::error,
				      conn,
				      invoker,
				      pack_size,
				      conflicts,
				      partial_ass
				      )
			  );

	///@todo TK: conflicts and partial_ass leak here

      }
  }


  ConflictNotification*
  wait_router(ConcurrentMessageQueue* router_neighbor_notif)
  {
    // wait for a notification from the Router
    ConflictNotification* cn = 0;
    void *ptr         = static_cast<void*>(&cn);
    unsigned int p    = 0;
    std::size_t recvd = 0;
    
    DMCS_LOG_TRACE("Listen to router...");
    router_neighbor_notif->receive(ptr, sizeof(cn), recvd, p);
	
    if (!ptr || !cn)
      {
	DMCS_LOG_FATAL("Got null message: " << ptr << " " << cn);
	assert(ptr != 0 && cn != 0);
      }

    return cn;
  }

  

  void
  handle_write_message(const boost::system::error_code& e,
		       connection_ptr conn,
		       std::size_t invoker,
		       std::size_t pack_size,
		       ConflictVec* conflicts, 
		       PartialBeliefState* partial_ass)
  {
    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

    if (!e)
      {
	StreamingForwardMessage mess(invoker, pack_size, conflicts, partial_ass);
	conn->async_write(mess,
			  boost::bind(&NeighborOut::handle_clean_up, this,  
				      boost::asio::placeholders::error,
				      conflicts,
				      partial_ass
				      )
			  );
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }



  void
  handle_clean_up(const boost::system::error_code& e,
		  ConflictVec* conflicts, 
		  PartialBeliefState* partial_ass)
  {
    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

    if (!e)
      {
	// We don't delete partial_ass here. It should be used in other places.

	///@todo TK: conflicts and partial_ass leak here

	for (ConflictVec::const_iterator it = conflicts->begin();
	     it != conflicts->end(); ++it)
	  {
	    Conflict* c = *it;
	    delete c;
	    c = 0;
	  }
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }


};

typedef boost::shared_ptr<NeighborOut> NeighborOutPtr;

} // namespace dmcs

#endif // NEIGHBOR_IN_H

// Local Variables:
// mode: C++
// End:
