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
 * @file   NewServer.cpp
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Sun Mar  11 20:28:30 2012
 * 
 * @brief  
 * 
 * 
 */

#include "network/HandlerWrapper.h"
#include "network/NewServer.h"

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

namespace dmcs {

void
HandlerWrapper::operator()(NewHandlerPtr handler,
			   connection_ptr conn,
			   NewConcurrentMessageDispatcherPtr md,
			   NewOutputDispatcherPtr od)
{
  handler->startup(handler, conn, md, od);
}

NewServer::NewServer(boost::asio::io_service& i,
		     const boost::asio::ip::tcp::endpoint& endpoint,
		     const RegistryPtr r)
  : io_service(i),
    io_service_to_manager(new boost::asio::io_service),
    acceptor(io_service, endpoint),
    port(endpoint.port()),
    reg(r),
    thread_factory(new NewThreadFactory),
    count_shutdown_handlers(0)
{
  first_initialization_phase();
  connection_ptr my_connection(new connection(io_service));

  DBGLOG(DBG, "NewServer::ctor: First phase initialization done. Waiting for new connection.");

  acceptor.async_accept(my_connection->socket(),
			boost::bind(&NewServer::handle_accept, this,
				    boost::asio::placeholders::error,
				    my_connection)
			);

  connect_to_manager();
}



NewServer::~NewServer()
{
  for (std::vector<HandlerWrapper*>::iterator it = handler_vec.begin();
       it != handler_vec.end(); ++it)
    {
      HandlerWrapper* handler = *it;
      assert (handler);

      delete handler;
      handler = 0;
    }

  for (std::vector<boost::thread*>::iterator it = handler_thread_vec.begin();
       it != handler_thread_vec.end(); ++it)
    {
      boost::thread* handler_thread = *it;
      assert (handler_thread);

      if (handler_thread->joinable())
	{
	  handler_thread->interrupt();
	  handler_thread->join();
	  delete handler_thread;
	  handler_thread = 0;
	}
    }
}



void
NewServer::first_initialization_phase()
{
  DBGLOG(DBG, "NewServer::initialize()");
  std::size_t no_neighbors = 0;
  if (reg->neighbors != NewNeighborVecPtr())
    {
      no_neighbors = reg->neighbors->size();
    }

  reg->belief_state_offset = BeliefStateOffset::create(reg->system_size, reg->belief_set_size);
  reg->message_dispatcher = NewConcurrentMessageDispatcherPtr(new NewConcurrentMessageDispatcher(reg->queue_size, no_neighbors));

  reg->request_dispatcher = RequestDispatcherPtr(new RequestDispatcher);
  reg->output_dispatcher = NewOutputDispatcherPtr(new NewOutputDispatcher);
}


void
NewServer::connect_to_manager()
{
  DBGLOG(DBG, "NewServer::connect_to_manager");

  boost::asio::ip::tcp::resolver resolver(*io_service_to_manager);
  boost::asio::ip::tcp::resolver::query query(reg->manager_hostname, reg->manager_port);
  boost::asio::ip::tcp::resolver::iterator res_it = resolver.resolve(query);
  boost::asio::ip::tcp::endpoint endpoint = *res_it;
  
  conn_to_manager = connection_ptr(new connection(*io_service_to_manager));
  
  conn_to_manager->socket().async_connect(endpoint,
					  boost::bind(&NewServer::send_notification_to_manager, this,
						      boost::asio::placeholders::error,
						      conn_to_manager,
						      reg->message_dispatcher,
						      ++res_it)
					  );
  
    boost::shared_ptr<boost::thread> mt(new boost::thread(boost::bind(&boost::asio::io_service::run, io_service_to_manager)));
    io_service_to_manager->run();
    
    mt->join(); // waits for termination
}



void
NewServer::notify_shutdown_handler() 
{
  ///@todo: mutex protection here
  count_shutdown_handlers++;
}



bool
NewServer::isShutdown()
{
  assert (count_shutdown_handlers <= handler_vec.size());
  return (count_shutdown_handlers == handler_vec.size());
}



void
NewServer::shutdown(NewConcurrentMessageDispatcherPtr md)
{
  int timeout = 0;
  ForwardMessage* end_mess = NULL;
  md->send(NewConcurrentMessageDispatcher::REQUEST_DISPATCHER_MQ, end_mess, timeout);
  
  ReturnedBeliefState* end_res = NULL;
  md->send(NewConcurrentMessageDispatcher::OUTPUT_DISPATCHER_MQ, end_res, timeout);
  
  if (reg->joiner_dispatcher != NewJoinerDispatcherPtr())
    {
      NewJoinIn* end_notif = NULL;
      md->send(NewConcurrentMessageDispatcher::JOINER_DISPATCHER_MQ, end_notif, timeout);
    }

  thread_factory->killThreads();

  // notify Manager
  std::string str_terminate = HEADER_TERMINATE;
  conn_to_manager->write(str_terminate);
  conn_to_manager->socket().close();
}


void
NewServer::send_notification_to_manager(const boost::system::error_code& e,
					connection_ptr conn,
					NewConcurrentMessageDispatcherPtr md,
					boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
  if (!e)
    {
      std::string notification = INIT_PHASE1_COMPLETED;
      conn->async_write(notification,
			boost::bind(&NewServer::wait_trigger, this,
				    boost::asio::placeholders::error,
				    conn));
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
	// Try the next endpoint.
	conn->socket().close();
	
	boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
	conn->socket().async_connect(endpoint,
				     boost::bind(&NewServer::send_notification_to_manager, this,
						 boost::asio::placeholders::error,
						 conn,
						 md,
						 ++endpoint_iterator)
				     );
    }
  else
    {
      DBGLOG(ERROR, "NewServer::send_notification_to_manager: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}


void
NewServer::wait_trigger(const boost::system::error_code& e,
			connection_ptr conn)
{
  if (!e)
    {
      boost::shared_ptr<std::string> trigger_message(new std::string);
      conn->async_read(*trigger_message,
			boost::bind(&NewServer::handle_read_trigger_message, this,
				    boost::asio::placeholders::error,
				    trigger_message,
				    conn));      
    }
  else
    {
      DBGLOG(ERROR, "NewServer::wait_trigger: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}


void
NewServer::handle_read_trigger_message(const boost::system::error_code& e,
				       boost::shared_ptr<std::string> trigger_message,
				       connection_ptr conn)
{
  if (!e)
    {
      DBGLOG(DBG, "NewServer::handle_read_trigger_message: got " << *trigger_message);
      if (trigger_message->compare(INIT_START_PHASE2) == 0)
	{
	  second_initialization_phase();
	}
      else
	{
	  DBGLOG(ERROR, "NewServer::handle_read_trigger_message: Unknown message from Manager.");
	  throw std::runtime_error("Unknown message from Manager");
	}
    }
  else
    {
      DBGLOG(ERROR, "NewServer::handle_read_trigger_message: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}


void
NewServer::second_initialization_phase()
{
  if (reg->neighbors != NewNeighborVecPtr())
    {
      DBGLOG(DBG, "NewServer::initialize(): intermediate context.");
      thread_factory->createNeighborThreads(reg->message_dispatcher,
					    reg->neighbors);

      reg->joiner_dispatcher = NewJoinerDispatcherPtr(new NewJoinerDispatcher);
    }
  
  DBGLOG(DBG, "NewServer::initialize(): create main threads.");
  thread_factory->createMainThreads(reg->message_dispatcher,
				    reg->request_dispatcher,
				    reg->output_dispatcher,
				    reg->joiner_dispatcher);

  DBGLOG(DBG, "NewServer::initialize(): create context threads.");
  thread_factory->createContextThreads(reg->contexts,
				       reg->message_dispatcher,
				       reg->request_dispatcher,
				       reg->joiner_dispatcher);
  DBGLOG(DBG, "NewServer::initialize(): second initialization phase DONE.");
}



void
NewServer::handle_accept(const boost::system::error_code& e, 
			 connection_ptr conn)
{
  if (!e)
    {
      // boost::this_thread::interruption_point();
      // Start an accept operation for a new connection.
      connection_ptr new_conn(new connection(acceptor.get_io_service()));
      acceptor.async_accept(new_conn->socket(),
			    boost::bind(&NewServer::handle_accept, this,
					boost::asio::placeholders::error,
					new_conn)
			    );

      boost::shared_ptr<std::string> header(new std::string);
      
      conn->async_read(*header,
		       boost::bind(&NewServer::handle_read_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   header));
    }
  else
    {
      DBGLOG(ERROR, "NewServer::handle_accept: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}



void
NewServer::handle_read_header(const boost::system::error_code& e, 
			      connection_ptr conn,
			      boost::shared_ptr<std::string> header)
{
  //boost::mutex::scoped_lock lock(mtx);
  if (!e)
    {
      DBGLOG(DBG, "NewServer: got header = " << *header);
      if (header->find(HEADER_REQ_DMCS) != std::string::npos)
	{
	  boost::asio::ip::tcp::socket& sock = conn->socket();
	  boost::asio::ip::tcp::endpoint ep  = sock.remote_endpoint(); 

	  

	  NewHandlerPtr handler(new NewHandler(ep.port(), this));
	  HandlerWrapper* handler_wrapper = new HandlerWrapper();

	  boost::this_thread::interruption_point();
	  assert (reg);
	  boost::thread* handler_thread = new boost::thread(*handler_wrapper,
							    handler,
							    conn,
							    reg->message_dispatcher,
							    reg->output_dispatcher);

	  handler_vec.push_back(handler_wrapper);
	  handler_thread_vec.push_back(handler_thread);
	}
    }
  else
    {
      DBGLOG(ERROR, "NewServer::handle_read_header: ERROR:" << e.message());
      throw std::runtime_error(e.message());
    }
}



} // namespace dmcs

// Local Variables:
// mode: C++
// End:
