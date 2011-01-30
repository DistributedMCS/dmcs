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

#ifndef NEIGHBOR_THREAD_H
#define NEIGHBOR_THREAD_H

#include "dmcs/ConflictNotification.h"
#include "dmcs/Log.h"
#include "dmcs/Neighbor.h"
#include "mcs/HashedBiMap.h"
#include "network/NeighborOut.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "network/connection.hpp"

#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/string.hpp>


namespace dmcs {

class NeighborThread
{
public:
  NeighborThread(std::size_t p)
    : nop_thread(0),
      io_service(new boost::asio::io_service),
      port(p)
  { }


  virtual
  ~NeighborThread()
  {
    DMCS_LOG_TRACE(port << ": Killing me softly");

    if (nop_thread)
      {
	nop_thread->interrupt();

	if (nop_thread->joinable())
	  {
	    nop_thread->join();
	    delete nop_thread;
	    nop_thread = 0;
	    DMCS_LOG_TRACE(port << ": Joined NeighborOut");
	  }
      }

  }


  void
  operator()(ConcurrentMessageQueue* rnn,
	     MessagingGatewayBC* mg_,
	     const Neighbor* nb_,
	     const HashedBiMap* c2o_,
	     const std::size_t invoker_,
	     const std::size_t pack_size_)
  {
    router_neighbor_notif = rnn;
    mg = mg_;
    nb = nb_; 
    c2o = c2o_;
    invoker = invoker_;
    pack_size = pack_size_;

    DMCS_LOG_TRACE(port << ": starting neighbor " << nb->neighbor_id << ": " << nb->port << "@" << nb->hostname);

    boost::asio::ip::tcp::resolver resolver(*io_service);
    boost::asio::ip::tcp::resolver::query query(nb->hostname, nb->port);
    boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
    boost::asio::ip::tcp::endpoint endpoint = *res_it;

    connection_ptr conn(new connection(*io_service));

    conn->socket().async_connect(endpoint,
				 boost::bind(&NeighborThread::establish_connections, this,
					     boost::asio::placeholders::error,
					     conn,
					     ++res_it)
				 );


    DMCS_LOG_TRACE(port << ": starting neighbor " << nb->neighbor_id << " io_service");

    boost::shared_ptr<boost::thread> nip(new boost::thread(boost::bind(&boost::asio::io_service::run, io_service)));

    nip->join(); // waits for termination

    DMCS_LOG_TRACE(port << ": io_service for neighbor " << nb->neighbor_id << " done");
  }



  void
  stop()
  {
    DMCS_LOG_TRACE(port << ": stopping io_service " << io_service);

    if (io_service)
      {
	io_service->stop(); // terminate io_service
	DMCS_LOG_TRACE(port << ": io_service stopped");
      }
  }


private:


  void
  establish_connections(const boost::system::error_code& e,
			connection_ptr conn,
			boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
  {
    if (!e)
      {
	//DMCS_LOG_TRACE("neighbor: ");
	//DMCS_LOG_TRACE(nb->neighbor_id << ": " << nb->port << "@" << nb->hostname);
	
	// get the offset of the neighbor
	const std::size_t nid                   = nb->neighbor_id;
	const HashedBiMapByFirst& from_context  = boost::get<Tag::First>(*c2o);
	HashedBiMapByFirst::const_iterator pair = from_context.find(nid);
	const std::size_t offset                = pair->second;
	
	
	NeighborOut nop(port);
	nop_thread = new boost::thread(nop,
				       conn,
				       router_neighbor_notif,
				       invoker,
				       nid,
				       pack_size);


	boost::shared_ptr<std::string> header(new std::string);
	
	// read from network
	DMCS_LOG_TRACE(port << ": Reading header from network, offset = " << offset);
	conn->async_read(*header,
			 boost::bind(&NeighborThread::handle_read_header, this,  
				     boost::asio::placeholders::error,
				     conn,
				     header,
				     offset
				     )
			 );
      }
    else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
      {
	// Try the next endpoint.
	conn->socket().close();
	
	boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
	conn->socket().async_connect(endpoint,
				     boost::bind(&NeighborThread::establish_connections, this,
						 boost::asio::placeholders::error,
						 conn,
						 ++endpoint_iterator)
				     );
      }
    else
      {
	// An error occurred.
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << port << ": " << e.message());
	throw std::runtime_error(e.message());
    }
  }  



 
  void
  handle_read_header(const boost::system::error_code& e,
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header,
		     std::size_t noff)
  {
    if (!e)
      {
	if (header->find(HEADER_ANS) != std::string::npos)
	  {
	    StreamingBackwardMessagePtr mess(new StreamingBackwardMessage);

	    // read some models
	    DMCS_LOG_TRACE(port << ": Reading message from network");

	    conn->async_read(*mess,
			     boost::bind(&NeighborThread::handle_read_message, this,
					 boost::asio::placeholders::error,
					 conn,
					 mess,
					 noff
					 )
			     );
	  }
	else
	  {
	    // no message to read, inform the Joiner with (ctx_offset, 0)
	    assert (header->find(HEADER_EOF) != std::string::npos);
	    mg->sendJoinIn(0, noff, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);

	    boost::shared_ptr<std::string> header(new std::string);
	    
	    // read from network
	    DMCS_LOG_TRACE(port << ": Reading header from network, noff = " << noff);
	    conn->async_read(*header,
			     boost::bind(&NeighborThread::handle_read_header, this,  
					 boost::asio::placeholders::error,
					 conn,
					 header,
					 noff
					 )
			     );
	  }
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << port << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }


  void
  handle_read_message(const boost::system::error_code& e,
		      connection_ptr conn,
		      StreamingBackwardMessagePtr mess,
		      std::size_t noff)
  {
    if (!e)
      {
	// extract a bunch of models from the message and then put each into NEIGHBOR_MQ
	DMCS_LOG_TRACE(port << ": Write to MQs. noff = " << noff);
	
	const PartialBeliefStateVecPtr& bsv = mess->getBeliefStates();
	
	DMCS_LOG_TRACE(port << ": Received " << bsv->size() << " partial belief states from noff = " << noff << ": \n" << *mess);
	
	const std::size_t offset = ConcurrentMessageQueueFactory::NEIGHBOR_MQ + noff;

	// notify the joiner by putting a JoinMess into JoinMessageQueue
	std::size_t bsv_size = bsv->size();
	mg->sendJoinIn(bsv_size, noff, ConcurrentMessageQueueFactory::JOIN_IN_MQ, 0);
	
	for (PartialBeliefStateVec::const_iterator it = bsv->begin();
	     it != bsv->end();
	     ++it)
	  {
	    PartialBeliefState* bs = *it;
	    if (bs != 0)
	      {
		DMCS_LOG_TRACE(port << ": Sending model: " << *bs);
	      }
	    else
	      {
		DMCS_LOG_TRACE(port << ": Sending model: NULL");
	      }
	    mg->sendModel(bs, noff, offset, 0);
	  }
	
	boost::shared_ptr<std::string> header(new std::string);
	
	// read from network
	DMCS_LOG_TRACE(port << ": Reading header from network, noff = " << noff);
	conn->async_read(*header,
			 boost::bind(&NeighborThread::handle_read_header, this,  
				     boost::asio::placeholders::error,
				     conn,
				     header,
				     noff
				     )
			 );
      }
    else
      {
	DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << port << ": " << e.message());
	throw std::runtime_error(e.message());
      }
  }



private:
  ConcurrentMessageQueue*    router_neighbor_notif;
  MessagingGatewayBC*        mg;
  const Neighbor*            nb;
  const HashedBiMap*         c2o;
  std::size_t                invoker;
  std::size_t                pack_size;
  boost::thread* nop_thread;
  boost::thread* nip_thread;
  boost::shared_ptr<boost::asio::io_service> io_service;
  std::size_t port;
};


  typedef std::vector<NeighborThread*> NeighborThreadVec;
  typedef boost::shared_ptr<NeighborThreadVec> NeighborThreadVecPtr;



} // namespace dmcs


#endif // NEIGHBOR_THREAD_H

// Local Variables:
// mode: C++
// End:
