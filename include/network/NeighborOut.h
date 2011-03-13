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
private:
  std::size_t port;

public:
  NeighborOut(std::size_t p)
    : port(p)
  { }


  virtual
  ~NeighborOut()
  {
    DMCS_LOG_TRACE(port << ": Terminating NeighborOut.");
  }


  void
  operator() (connection_ptr conn,
	      ConcurrentMessageQueue* rnn,
	      std::size_t invoker, 
	      std::size_t nid)
  {
    while (1)
      {
	ConflictNotification* cn = wait_router(rnn, nid);

	std::size_t session_id = cn->session_id;
	std::size_t pack_size = cn->pack_size;

	assert (pack_size > 0);

	std::string header;

	if (cn->type == ConflictNotification::SHUTDOWN)
	  {
	    DMCS_LOG_TRACE(port << ": Neighbor " << nid << " received SHUTDOWN, propagating TERMINATE...");

	    header = HEADER_TERMINATE;

	    conn->write(header);

	    //delete cn;
	    //cn = 0;

	    return; // done with looping
	  }
	else if (cn->type == ConflictNotification::REQUEST)
	  {
	    header = HEADER_REQ_STM_DMCS;
	  }
	else if (cn->type == ConflictNotification::NEXT)
	  {
	    // We should only send HEADER_NEXT
	    boost::asio::ip::tcp::socket& sock = conn->socket();
	    boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
	    DMCS_LOG_TRACE(port << ": Going to send HEADER_NEXT to port " << ep.port());

	    header = HEADER_NEXT;
	  }
	else
	  {
	    assert(false && "should not come here");
	  }

	//delete cn;
	//cn = 0;

	conn->write(header);

	write_message(conn,
		      invoker,
		      pack_size,
		      session_id);
      }
  }

  ConflictNotification*
  wait_router(ConcurrentMessageQueue* router_neighbor_notif, std::size_t nid)
  {
    // wait for a notification from the Router
    ConflictNotification* cn = 0;
    void *ptr         = static_cast<void*>(&cn);
    unsigned int p    = 0;
    std::size_t recvd = 0;
    
    DMCS_LOG_TRACE(port << ": Listen to router... nid = " << nid);
    router_neighbor_notif->receive(ptr, sizeof(cn), recvd, p);
	
    if (!ptr || !cn)
      {
	DMCS_LOG_FATAL("Got null message: " << ptr << " " << cn);
	assert(ptr != 0 && cn != 0);
      }

    return cn;
  }

  

  void
  write_message(connection_ptr conn,
		std::size_t invoker,
		std::size_t pack_size,
		std::size_t session_id)
  {
    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

    StreamingForwardMessage mess(session_id,
				 invoker,
				 pack_size);

    conn->write(mess);
  }
  

};

typedef boost::shared_ptr<NeighborOut> NeighborOutPtr;

} // namespace dmcs

#endif // NEIGHBOR_IN_H

// Local Variables:
// mode: C++
// End:
