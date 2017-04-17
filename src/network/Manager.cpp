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
 * @file   Manager.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Thu Oct  25 22:37:24 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/Logger.h"
#include "mcs/NewMessage.h"
#include "network/Manager.h"

namespace dmcs {

Manager::Manager(boost::asio::io_service& i,
		 const boost::asio::ip::tcp::endpoint& endpoint,
		 std::size_t system_size)
  : io_service(i),
    acceptor(io_service, endpoint),
    system_size(system_size),
    count_terminations(0)
{ 
  connection_ptr my_connection(new connection(io_service));

  DBGLOG(DBG, "Manager::ctor: waiting for new connection.");

  acceptor.async_accept(my_connection->socket(),
			boost::bind(&Manager::handle_accept, this,
				    boost::asio::placeholders::error,
				    my_connection)
			);
}



void
Manager::handle_accept(const boost::system::error_code& e, 
		       connection_ptr conn)
{
  if (!e)
    {
      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor.get_io_service()));
      acceptor.async_accept(new_conn->socket(),
			    boost::bind(&Manager::handle_accept, this,
					boost::asio::placeholders::error,
					new_conn)
			    );

      boost::shared_ptr<std::string> header(new std::string);
      
      conn->async_read(*header,
		       boost::bind(&Manager::handle_read_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   header));
    }
  else
    {
      DBGLOG(ERROR, "Manager::handle_accept: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    } 
}



void
Manager::handle_read_header(const boost::system::error_code& e, 
			    connection_ptr conn,
			    boost::shared_ptr<std::string> header)
{
  boost::mutex::scoped_lock lock(mtx);

  if (!e)
    {
      DBGLOG(DBG, "Manager::handle_read_header(): got header = " << *header);
      if (header->compare(INIT_PHASE1_COMPLETED) == 0)
	{
	  connections_vec.push_back(conn);
	  if (connections_vec.size() == system_size)
	    {
	      trigger_2nd_phase();
	    }
	}
      else
	{
	  DBGLOG(ERROR, "Manager::handle_read_header(): Unknown notification from dmcsd.");
	  throw std::runtime_error("Unknown notification from dmcsd.");
	}
    }
  else
    {
      DBGLOG(ERROR, "Manager::handle_read_header(): ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}


void
Manager::trigger_2nd_phase()
{
  std::string trigger_message = INIT_START_PHASE2;
  for (std::vector<connection_ptr>::const_iterator it = connections_vec.begin(); it != connections_vec.end(); ++it)
    {
      connection_ptr conn = *it;
      conn->async_write(trigger_message,
			boost::bind(&Manager::wait_termination, this,
				   boost::asio::placeholders::error,
				    conn)
			);
    }
}


void
Manager::wait_termination(const boost::system::error_code& e, 
			  connection_ptr conn)
{
  if (!e)
    {
      // do nothing here as we are struggling to propagate the end_mess to neighbors,
      // hence there will not be enough notifications of termination to the Manager.

      /*boost::shared_ptr<std::string> header(new std::string);
      
      conn->async_read(*header,
		       boost::bind(&Manager::handle_finalize, this,
				   boost::asio::placeholders::error,
				   conn,
				   header));*/
    }
  else
    {
      DBGLOG(ERROR, "Manager::wait_termination: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}



void
Manager::handle_finalize(const boost::system::error_code& e, 
			 connection_ptr conn,
			 boost::shared_ptr<std::string> header)
{
  boost::mutex::scoped_lock lock(mtx);

  if (!e)
    {
      DBGLOG(DBG, "Manager::handle_finalize(): got header = " << *header);
      if (header->compare(HEADER_TERMINATE) == 0)
	{
	  count_terminations++;
	  if (count_terminations == system_size)
	    {
	      close_all_connections();
	    }
	}
      else
	{
	  DBGLOG(ERROR, "Manager::handle_finalize(): Unknown header from dmcsd.");
	  throw std::runtime_error("Unknown header from dmcsd.");
	}
    }
  else
    {
      DBGLOG(ERROR, "Manager::handle_finalize: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}



void
Manager::close_all_connections()
{
  for (std::vector<connection_ptr>::const_iterator it = connections_vec.begin(); it != connections_vec.end(); ++it)
    {
      connection_ptr conn = *it;
      conn->socket().close();
    }
}


} // namespace dmcs


// Local Variables:
// mode: C++
// End:
