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
 * @file   Server.cpp
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Tue Mar  2 07:17:31 2010
 * 
 * @brief  
 * 
 * 
 */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <iostream>

#include "network/Handler.h"
#include "network/Server.h"


namespace dmcs {


Server::Server(CommandTypeFactoryPtr& ctf_,
	       boost::asio::io_service& io_service,
	       const boost::asio::ip::tcp::endpoint& endpoint)
  : ctf(ctf_), io_service_(io_service),
    acceptor_(io_service, endpoint)
{
  connection_ptr my_connection(new connection(io_service_));

  acceptor_.async_accept(my_connection->socket(),
			 boost::bind(&Server::handle_accept, this,
				     boost::asio::placeholders::error, my_connection));
}



void
Server::handle_accept(const boost::system::error_code& error, connection_ptr conn)
{
  if (!error)
    {
#if defined(DEBUG)
      std::cerr << "Server::handle_accept" << std::endl 
		<< "Creating new connection..." << std::endl;
#endif // DEBUG

      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor_.io_service()));
      acceptor_.async_accept(new_conn->socket(),
			     boost::bind(&Server::handle_accept, this,
					 boost::asio::placeholders::error, new_conn));

#if defined(DEBUG)
      std::cerr << "Expect some header" << std::endl;
#endif // DEBUG

      // read header to decide what kind of command type to create
      conn->async_read(header, boost::bind(&Server::dispatch_header, this,
					   boost::asio::placeholders::error, 
					   conn));
    }
  else
    {
      // An error occurred. Log it and return. Since we are not starting a new
      // accept operation the io_service will run out of work to do and the
      // server will exit.

#ifdef DEBUG
      std::cerr << "handle_accept: " << error.message() << std::endl;
#endif
    }
}


void
Server::dispatch_header(const boost::system::error_code& e, connection_ptr conn)
{
  if (!e)
    {
#ifdef DEBUG
      std::cerr << "Server::dispatch_header" << std::endl;
#endif // DEBUG
      
#ifdef DEBUG
      std::cerr << "Header = \"" << header << "\"" << std::endl;
#endif
      
      // Create the respective handler and give him the connection
      if (header.find(HEADER_REQ_PRI_DMCS) != std::string::npos)
	{
	  PrimitiveCommandTypePtr cmt_pri_dmcs = ctf->create<PrimitiveCommandTypePtr>();
	  Handler<PrimitiveCommandType> handler(cmt_pri_dmcs, conn);
	}
      else if (header.find(HEADER_REQ_STM_DMCS) != std::string::npos)
	{
	  StreamingCommandTypePtr cmt_stm_dmcs = ctf->create<StreamingCommandTypePtr>();
	  Handler<StreamingCommandType> handler(cmt_stm_dmcs, conn);
	}
      else if (header.find(HEADER_REQ_OPT_DMCS) != std::string::npos)
	{
	  OptCommandTypePtr cmt_opt_dmcs = ctf->create<OptCommandTypePtr>();
	  Handler<OptCommandType> handler(cmt_opt_dmcs, conn);
	}
      else if (header.find(HEADER_REQ_DYN_DMCS) != std::string::npos)
	{
	  DynamicCommandTypePtr cmt_dyn_conf = ctf->create<DynamicCommandTypePtr>();
	  Handler<DynamicCommandType> handler(cmt_dyn_conf, conn);
	}
      else if (header.find(HEADER_REQ_INSTANTIATE) != std::string::npos)
	{
	  InstantiatorCommandTypePtr cmt_inst = ctf->create<InstantiatorCommandTypePtr>();
	  Handler<InstantiatorCommandType> handler(cmt_inst, conn);
	}
    }
  else
    {
      // An error occurred.
      
#ifdef DEBUG
      std::cerr << "Server::dispatch_header: " << e.message() << std::endl;
#endif
    }
}
  

void
Server::handle_finalize(const boost::system::error_code& e, connection_ptr /* conn */)
{
  // Nothing to do. The socket will be closed automatically when the last
  // reference to the connection object goes away.
#ifdef DEBUG
  std::cerr << "in handle_finalize: " << std::endl;
#endif
  if (e)
    {
      // An error occurred.

#ifdef DEBUG
      std::cerr << "handle_finalize: " << e.message() << std::endl;
#endif
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
