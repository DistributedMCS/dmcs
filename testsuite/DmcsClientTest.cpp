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
 * @file   DmcsClientTest.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Mar  13 9:59:02 2012
 * 
 * @brief  
 * 
 * 
 */

#include <sstream>
#include "DmcsClientTest.h"
#include "mcs/QueryID.h"

using namespace dmcs;

DmcsClientTest::DmcsClientTest(boost::asio::io_service& i,
			       boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			       ForwardMessage* ws1,
			       ForwardMessage* ws2)
  : io_service(i),
    conn(new connection(io_service)),
    count(0)
{
  std::cerr << "DmcsClientTest: connecting to server..." << std::endl;
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
  conn->socket().async_connect(endpoint,
			      boost::bind(&DmcsClientTest::send_header, this,
					  boost::asio::placeholders::error,
					  ++endpoint_iterator,
					  conn,
					  ws1,
					  ws2));
}



void
DmcsClientTest::send_header(const boost::system::error_code& e,
			    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
			    connection_ptr conn,
			    ForwardMessage* ws1,
			    ForwardMessage* ws2)
{
  if (!e)
    {
      std::string header = HEADER_REQ_DMCS;
      std::cerr << "DmcsClientTest: sending header = " << header << std::endl;
      conn->async_write(header,
			boost::bind(&DmcsClientTest::send_message, this,
				    boost::asio::placeholders::error,
				    conn,
				    ws1,
				    ws2));
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      conn->socket().close();
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

      conn->socket().async_connect(endpoint,
				   boost::bind(&DmcsClientTest::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator,
					       conn,
					       ws1,
					       ws2));
    }
  else
    {
      std::cerr << "DmcsClientTest::send_header(): ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
DmcsClientTest::send_message(const boost::system::error_code& e,
			     connection_ptr conn,
			     ForwardMessage* ws1,
			     ForwardMessage* ws2)
{
  if (!e)
    {
      ForwardMessage* to_be_sent;
      if (count == 0)
	{
	  to_be_sent = ws1;
	}
      else
	{
	  to_be_sent = ws2;
	}
      ++count;

      std::cerr << "DmcsClientTest: sending message = " << *to_be_sent << std::endl;
      conn->async_write(*to_be_sent,
			boost::bind(&DmcsClientTest::read_header, this,
				    boost::asio::placeholders::error,
				    conn,
				    ws1,
				    ws2));
    }
  else
    {
      std::cerr << "DmcsClientTest::send_message(): ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}


void
DmcsClientTest::read_header(const boost::system::error_code& e,
			    connection_ptr conn,
			    ForwardMessage* ws1,
			    ForwardMessage* ws2)
{
  std::cerr << "DmcsClientTest::read_header" << std::endl;
  if (!e)
    {
      boost::shared_ptr<std::string> header(new std::string);
      std::cerr << "DmcsClientTest: going to read" << std::endl;
      conn->async_read(*header,
		       boost::bind(&DmcsClientTest::handle_read_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   header,
				   ws1,
				   ws2));
    }
  else
    {
      std::cerr << "DmcsClientTest::read_header(): ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}


void
DmcsClientTest::handle_read_header(const boost::system::error_code& e,
				   connection_ptr conn,
				   boost::shared_ptr<std::string> header,
				   dmcs::ForwardMessage* ws1,
				   dmcs::ForwardMessage* ws2)
{
  std::cerr << "DmcsClientTest: handle_read_header = " << std::endl;
  if (!e)
    {
      assert (header->find(HEADER_ANS) != std::string::npos);
      std::cerr << "DmcsClientTest: Got header = " << *header << std::endl;

      BackwardMessagePtr bmess(new BackwardMessage);
      
      conn->async_read(*bmess,
		       boost::bind(&DmcsClientTest::handle_read_message, this,
				   boost::asio::placeholders::error,
				   conn,
				   bmess,
				   ws1,
				   ws2));
    }
  else
    {
      std::cerr << "DmcsClientTest::handle_read_reader(): ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}


void
DmcsClientTest::handle_read_message(const boost::system::error_code& e,
				    connection_ptr conn,
				    BackwardMessagePtr bmess,
				    dmcs::ForwardMessage* ws1,
				    dmcs::ForwardMessage* ws2)
{
  if (!e)
    {
      std::cerr << "DmcsClientTest: Got " << bmess->rbsl->size() << " answers." << std::endl;
      if (count == 1)
	{
	   boost::system::error_code e;
	   boost::asio::ip::tcp::resolver::iterator it;
	   send_header(e, it, conn, ws1, ws2);	   
	}
      else
	{
	  std::cerr << "DmcsClientTest: FINISHED HERE!" << std::endl;
	  std::string header = HEADER_TERMINATE;

	  conn->async_write(header,
			    boost::bind(&DmcsClientTest::handle_finalize, this,
					boost::asio::placeholders::error,
					conn));
	}
    }
  else
    {
      std::cerr << "DmcsClientTest::handle_read_message(): ERROR: " << e.message() << std::endl;
      throw std::runtime_error(e.message());
    }
}



void
DmcsClientTest::handle_finalize(const boost::system::error_code& e,
				connection_ptr conn)
{
  if (!e)
    {
      std::cerr << "DmcsClientTest: Send shutdown_qid to neighbor!" << std::endl;
      std::size_t ctx_id = 0;
      std::size_t neighbor_id = 1;
      std::size_t sqid = shutdown_query_id(ctx_id, neighbor_id);
      ForwardMessage* end_mess = new ForwardMessage(sqid, 0, 0);
      conn->async_write(*end_mess,
			boost::bind(&DmcsClientTest::closing, this,
				    boost::asio::placeholders::error,
				    conn));
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}


void
DmcsClientTest::closing(const boost::system::error_code& e,
			connection_ptr conn)
{
  if (!e)
    {
      conn->socket().close();
    }
  else
    {
      throw std::runtime_error(e.message());
    }
}

// Local Variables:
// mode: C++
// End:
