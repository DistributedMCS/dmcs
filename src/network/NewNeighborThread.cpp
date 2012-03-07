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
 * @file   NewNeighborThread.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Mar  1 14:57:30 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/JoinIn.h"
#include "network/NewNeighborOut.h"
#include "network/NewNeighborThread.h"

#include <boost/bind.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/shared_ptr.hpp>

namespace dmcs {

NewNeighborThread::NewNeighborThread(NewNeighborPtr n)
  : neighbor(n),
    nop_thread(NULL),
    io_service(new boost::asio::io_service)
{ }



NewNeighborThread::~NewNeighborThread()
{
  if (nop_thread)
    {
      nop_thread->interrupt();
      if (nop_thread->joinable())
	{
	  nop_thread->join();
	  delete nop_thread;
	  nop_thread = 0;
	}
    }
}



void
NewNeighborThread::operator()(NewConcurrentMessageDispatcherPtr md)
{
  boost::asio::ip::tcp::resolver resolver(*io_service);
  boost::asio::ip::tcp::resolver::query query(neighbor->hostname, neighbor->port);
  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *res_it;
  
  connection_ptr conn(new connection(*io_service));
  
  conn->socket().async_connect(endpoint,
			       boost::bind(&NewNeighborThread::establish_connections, this,
					   boost::asio::placeholders::error,
					   conn,
					   md,
					   ++res_it)
			       );
  
    boost::shared_ptr<boost::thread> nip(new boost::thread(boost::bind(&boost::asio::io_service::run, io_service)));
    
    nip->join(); // waits for termination
}


void
NewNeighborThread::establish_connections(const boost::system::error_code& e,
					 connection_ptr conn,
					 NewConcurrentMessageDispatcherPtr md,
					 boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  if (!e)
    {
      NewNeighborOut nop;
      nop_thread = new boost::thread(nop, conn, md, neighbor->neighbor_offset);

      boost::shared_ptr<std::string> header(new std::string);

      conn->async_read(*header,
		       boost::bind(&NewNeighborThread::handle_read_header, this,  
				   boost::asio::placeholders::error,
				   conn,
				   md,
				   header));      
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
      {
	// Try the next endpoint.
	conn->socket().close();
	
	boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
	conn->socket().async_connect(endpoint,
				     boost::bind(&NewNeighborThread::establish_connections, this,
						 boost::asio::placeholders::error,
						 conn,
						 md,
						 ++endpoint_iterator)
				     );
      }
  else
    {
      throw std::runtime_error(e.message());
    }
}



void
NewNeighborThread::handle_read_header(const boost::system::error_code& e,
				      connection_ptr conn,
				      NewConcurrentMessageDispatcherPtr md,
				      boost::shared_ptr<std::string> header)
{
  if (!e)
    {
      assert ((header->find(HEADER_ANS) != std::string::npos) || 
	      (header->find(HEADER_TERMINATE) != std::string::npos));

      if (header->find(HEADER_TERMINATE) != std::string::npos)
	{
	  conn->socket().close();
	}
      else
	{
	  BackwardMessagePtr mess(new BackwardMessage);
	  
	  conn->async_read(*mess,
			   boost::bind(&NewNeighborThread::handle_read_message, this,
				       boost::asio::placeholders::error,
				       conn,
				       md,
				       mess));
	}
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}



void
NewNeighborThread::handle_read_message(const boost::system::error_code& e,
				       connection_ptr conn,
				       NewConcurrentMessageDispatcherPtr md,
				       BackwardMessagePtr mess)
{
  if (!e)
    {
      int timeout = 0;
      ReturnedBeliefStateListPtr result_list = mess->rbsl;
      NewJoinIn* nji = new NewJoinIn(neighbor->neighbor_offset, result_list->size());

      md->send(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, nji, timeout);

      for (ReturnedBeliefStateList::const_iterator it = result_list->begin();
	   it != result_list->end(); ++it)
	{
	  ReturnedBeliefState* rbs = *it;
	  assert (rbs);
	  md->send(NewConcurrentMessageDispatcher::NEIGHBOR_IN_MQ, neighbor->neighbor_offset, rbs, timeout);
	}

      boost::this_thread::interruption_point();
      boost::shared_ptr<std::string> header(new std::string);

      conn->async_read(*header,
		       boost::bind(&NewNeighborThread::handle_read_header, this,  
				   boost::asio::placeholders::error,
				   conn,
				   md,
				   header));   
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
