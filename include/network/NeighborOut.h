/* DMC -- Distributed Nonmonotonic Multi-Context Systems.
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

#include "dmcs/AskNextNotification.h"
#include "dmcs/Log.h"
#include "dmcs/StreamingForwardMessage.h"
#include "network/connection.hpp"

#include <boost/functional/hash.hpp>
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



  virtual
  ~NeighborOut()
  {
    DMCS_LOG_TRACE("Terminating NeighborOut.");
  }



  void
  operator() (connection_ptr conn,
	      ConcurrentMessageQueue* rnn,
	      std::size_t invoker, 
	      std::size_t nid)
  {
    while (1)
      {
	AskNextNotification* ann = wait_next_request(rnn, nid);

	DMCS_LOG_TRACE("N[" << nid << "] Got a message = " << *ann);

	std::size_t path = ann->path;
	std::size_t session_id = ann->session_id;
	std::size_t k1 = ann->k1;
	std::size_t k2 = ann->k2;

	assert (k1 <= k2);

	std::string header;

	if (ann->type == BaseNotification::SHUTDOWN)
	  {
	    DMCS_LOG_TRACE("N[" << nid << "] Received SHUTDOWN, propagating TERMINATE...");

	    header = HEADER_TERMINATE;

	    conn->write(header);

	    delete ann;
	    ann = 0;

	    return; // done with looping
	  }
	else if (ann->type == BaseNotification::REQUEST)
	  {
	    header = HEADER_REQ_STM_DMCS;
	  }
	else if (ann->type == BaseNotification::NEXT)
	  {
	    // We should only send HEADER_NEXT
	    boost::asio::ip::tcp::socket& sock = conn->socket();
	    boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 
	    DMCS_LOG_TRACE("N[" << nid << "] Going to send HEADER_NEXT to port " << ep.port());

	    header = HEADER_NEXT;
	  }
	else
	  {
	    assert(false && "should not come here");
	  }

	delete ann;
	ann = 0;

	conn->write(header);

	write_message(conn,
		      path,
		      invoker,
		      k1,
		      k2,
		      session_id);
      }
  }

  AskNextNotification*
  wait_next_request(ConcurrentMessageQueue* neighbor_notif, std::size_t nid)
  {
    AskNextNotification* ann = 0;
    void *ptr = static_cast<void*>(&ann);
    unsigned int p  = 0;
    std::size_t recvd = 0;
    
    DMCS_LOG_TRACE("N[" << nid << "] Listen to Joiner");
    neighbor_notif->receive(ptr, sizeof(ann), recvd, p);
	
    if (!ptr || !ann)
      {
	DMCS_LOG_FATAL("N[" << nid << "] Got null message: " << ptr << " " << ann);
	assert(ptr != 0 && ann != 0);
      }

    return ann;
  }

  

  void
  write_message(connection_ptr conn,
		std::size_t path,
		std::size_t invoker,
		std::size_t k1,
		std::size_t k2,
		std::size_t session_id)
  {
    DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

    boost::hash_combine(path, invoker);

    StreamingForwardMessage mess(path,
				 invoker,
				 session_id,
				 k1,
				 k2);

    conn->write(mess);
  }
  

};

typedef boost::shared_ptr<NeighborOut> NeighborOutPtr;

} // namespace dmcs

#endif // NEIGHBOR_IN_H

// Local Variables:
// mode: C++
// End:
