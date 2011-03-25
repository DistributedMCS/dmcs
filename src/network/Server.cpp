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

#include "dmcs/Log.h"

#include "network/Server.h"
#include "network/ConcurrentMessageQueueFactory.h"
#include "network/Handler.h"
#include "network/ThreadFactory.h"


#include <algorithm>
#include <iostream>
#include <string>


namespace dmcs {

  void
  HandlerThread::operator()(StreamingHandlerPtr handler,
			    StreamingSessionMsgPtr sesh,
			    MessagingGatewayBC* mg)
  {
    handler->startup(handler, sesh, mg);
  }


Server::Server(const CommandTypeFactoryPtr& c,
	       boost::asio::io_service& i,
	       const boost::asio::ip::tcp::endpoint& endpoint)
  : ctf(c),
    io_service(i),
    acceptor(io_service, endpoint),
    port(endpoint.port()),
    invokers(new ListSizeT),
    neighbors(new NeighborThreadVec),
    handlers(new HandlerThreadVec),
    neighbor_threads(new ThreadVec),
    handler_threads(new ThreadVec),
    joiner_sat_notif(new ConcurrentMessageQueue),
    neighbors_notif(new ConcurrentMessageQueueVec),
    first_round(true)
{
  connection_ptr my_connection(new connection(io_service));

  acceptor.async_accept(my_connection->socket(),
			boost::bind(&Server::handle_accept, this,
				    boost::asio::placeholders::error,
				    my_connection)
				    );
}



void
Server::initialize()
{
  StreamingCommandTypePtr cmd_stm_dmcs = ctf->create<StreamingCommandTypePtr>();

  const ContextPtr& ctx = ctf->getContext();
  const TheoryPtr& theory = ctf->getTheory();
  const QueryPlanPtr& query_plan = ctf->getQueryPlan();

  const SignaturePtr& local_sig = ctx->getSignature();
  std::size_t my_id = ctx->getContextID();
  std::size_t mq_size = ctf->getMQSize();

  const NeighborListPtr& nbs = query_plan->getNeighbors(my_id);
  std::size_t no_nbs = nbs->size();

  ConcurrentMessageQueueFactory& mqf = ConcurrentMessageQueueFactory::instance();
  mg = mqf.createMessagingGateway(port, no_nbs, mq_size);

  HashedBiMapPtr c2o(new HashedBiMap);

  // fill up the map: ctx_id <--> offset
  std::size_t off = 0;
  for (NeighborList::const_iterator it = nbs->begin(); it != nbs->end(); ++it, ++off)
    {
      std::size_t nid = (*it)->neighbor_id;
      c2o->insert(Int2Int(nid, off));
    }

  ThreadFactory tf(ctx, theory, local_sig, nbs, 0, joiner_sat_notif, mg.get(), c2o.get());
  
  if (no_nbs > 0)
    {
      DMCS_LOG_TRACE("Create Neighbor threads");
      tf.createNeighborThreads(neighbor_threads, neighbors, neighbors_notif);
      join_thread = tf.createJoinThread(neighbors_notif);
    }

  DMCS_LOG_TRACE("Create SAT thread");
  sat_thread = tf.createLocalSolveThread();

  DMCS_LOG_TRACE("All thread created!");
}



void
Server::handle_accept(const boost::system::error_code& e, connection_ptr conn)
{
  if (!e)
    {
      DMCS_LOG_TRACE("Creating new connection...");

      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor.io_service()));
      acceptor.async_accept(new_conn->socket(),
			    boost::bind(&Server::handle_accept, this,
					boost::asio::placeholders::error,
					new_conn)
					);

      DMCS_LOG_TRACE("Wait for header...");

      boost::shared_ptr<std::string> header(new std::string);

      // read header to decide what kind of command type to create
      conn->async_read(*header,
		       boost::bind(&Server::dispatch_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   header)
		       );
    }
  else
    {
      // An error occurred. Log it and return. Since we are not starting a new
      // accept operation the io_service will run out of work to do and the
      // server will exit.

      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



void
Server::dispatch_header(const boost::system::error_code& e,
			connection_ptr conn,
			boost::shared_ptr<std::string> header)
{
  if (!e)
    {
      DMCS_LOG_TRACE("Header = " << *header);

      // Create the respective handler and give him the connection
      if (header->find(HEADER_REQ_PRI_DMCS) != std::string::npos)
	{
	  typedef Handler<PrimitiveCommandType> PrimitiveHandler;

	  PrimitiveCommandTypePtr cmt_pri_dmcs = ctf->create<PrimitiveCommandTypePtr>();

	  PrimitiveHandler::SessionMsgPtr sesh(new PrimitiveHandler::SessionMsg(conn));

	  PrimitiveHandler::HandlerPtr handler(new PrimitiveHandler);

	  handler->start(handler, sesh, cmt_pri_dmcs);
	}
      else if (header->find(HEADER_REQ_STM_DMCS) != std::string::npos)
	{
	  if (first_round)
	    {
	      initialize();
	      first_round = false;
	    }

	  //typedef Handler<StreamingCommandType> StreamingHandler;

	  //StreamingCommandTypePtr cmd_stm_dmcs = ctf->create<StreamingCommandTypePtr>();

	  DMCS_LOG_TRACE("Going to create StreamingHandler");

	  StreamingSessionMsgPtr sesh(new StreamingSessionMsg(conn));

	  StreamingHandlerPtr handler(new StreamingHandler);

	  const std::size_t invoker = sesh->mess.getInvoker();
	  ListSizeT::const_iterator it = std::find(invokers->begin(), invokers->end(), invoker);

	  DMCS_LOG_TRACE("Invoker = " << invoker);
	  if (it != invokers->end())
	    {
	      DMCS_LOG_TRACE("WARNING: more than one connection created for invoker " << invoker);
	    }
	  else
	    {
	      invokers->push_back(invoker);
	    }

	  DMCS_LOG_TRACE(handler.get());
	  HandlerThread* handler_thread = new HandlerThread(invoker);
	  boost::thread* ht = new boost::thread(*handler_thread, handler, sesh, mg.get());
	  handler_threads->push_back(ht);
	  handlers->push_back(handler_thread);
	  ///@todo: delete threads in destructor of Server
	}
      else if (header->find(HEADER_REQ_OPT_DMCS) != std::string::npos)
	{
	  typedef Handler<OptCommandType> OptHandler;

	  OptCommandTypePtr cmt_opt_dmcs = ctf->create<OptCommandTypePtr>();

	  OptHandler::SessionMsgPtr sesh(new OptHandler::SessionMsg(conn));

	  OptHandler::HandlerPtr handler(new OptHandler);

	  handler->start(handler, sesh, cmt_opt_dmcs);
	}
      else if (header->find(HEADER_REQ_DYN_DMCS) != std::string::npos)
	{
	  typedef Handler<DynamicCommandType> DynHandler;

	  DynamicCommandTypePtr cmt_dyn_conf = ctf->create<DynamicCommandTypePtr>();

	  DynHandler::SessionMsgPtr sesh(new DynHandler::SessionMsg(conn));

	  DynHandler::HandlerPtr handler(new DynHandler);

	  handler->start(handler, sesh, cmt_dyn_conf);
	}
      else if (header->find(HEADER_REQ_INSTANTIATE) != std::string::npos)
	{
	  typedef Handler<InstantiatorCommandType> InstHandler;

	  InstantiatorCommandTypePtr cmt_inst = ctf->create<InstantiatorCommandTypePtr>();

	  InstHandler::SessionMsgPtr sesh(new InstHandler::SessionMsg(conn));

	  InstHandler::HandlerPtr handler(new InstHandler);

	  handler->start(handler, sesh, cmt_inst);
	}
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


void
Server::handle_finalize(const boost::system::error_code& e, connection_ptr /* conn */)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      // nothing to do
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}

} // namespace dmcs

// Local Variables:
// mode: C++
// End:
