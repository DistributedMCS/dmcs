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
 * @file   NewClient.cpp
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Mar  20 17:12:06 2012
 * 
 * @brief  
 * 
 * 
 */

#include "mcs/Logger.h"
#include "network/NewClient.h"

namespace dmcs {

void
NewClient::setCallback(CallbackFunc f)
{
  callback = f;
}



NewClient::NewClient(boost::asio::io_service& io_service,
		     boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		     const std::string& h,
		     ForwardMessage& fwd_mess)
  : conn(new connection(io_service)),
    callback(0)
{
  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

  boost::shared_ptr<std::string> header(new std::string(h));

  ForwardMessagePtr mess(new ForwardMessage(fwd_mess));

  conn->socket().async_connect(endpoint,
			       boost::bind(&NewClient::send_header, this,
					   boost::asio::placeholders::error,
					   ++endpoint_iterator,
					   conn,
					   header,
					   mess));  
}



void
NewClient::send_header(const boost::system::error_code& e,
		       boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		       connection_ptr conn,
		       boost::shared_ptr<std::string> header,
		       ForwardMessagePtr mess)
{
  if (!e)
    {
      DBGLOG(DBG, "NewClient::send_header(): header = " << *header);
      conn->async_write(*header,
			boost::bind(&NewClient::send_message, this,
				    boost::asio::placeholders::error,
				    conn,
				    header,
				    mess));      
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      conn->socket().close();
      
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      conn->socket().async_connect(endpoint,
				   boost::bind(&NewClient::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator,
					       conn,
					       header,
					       mess));
    }
  else
    {
      DBGLOG(ERROR, "NewClient::send_header(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }
}



void
NewClient::send_message(const boost::system::error_code& e,
			connection_ptr conn,
			boost::shared_ptr<std::string> header,
			ForwardMessagePtr mess)
{
  if (!e)
    {
      DBGLOG(DBG, "NewClient::send_message(): mess = " << *mess);
      conn->async_write(*mess,
			boost::bind(&NewClient::read_header, this,
				    boost::asio::placeholders::error,
				    conn,
				    header,
				    mess));      
    }
  else
    {
      DBGLOG(ERROR, "NewClient::send_message(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }
}


void
NewClient::read_header(const boost::system::error_code& e,
		       connection_ptr conn,
		       boost::shared_ptr<std::string> header,
		       ForwardMessagePtr mess)
{
  if (!e)
    {
      conn->async_read(*header,
		       boost::bind(&NewClient::handle_read_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   header,
				   mess));
    }
  else
    {
      DBGLOG(ERROR, "NewClient::read_header(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }  
}



void 
NewClient::handle_read_header(const boost::system::error_code& e, 
			      connection_ptr conn,
			      boost::shared_ptr<std::string> header,
			      ForwardMessagePtr mess)
{
  if (!e)
    {
      DBGLOG(DBG, "NewClient::handle_read_header(): Got header = " << *header);
      if (header->compare(HEADER_EOF) == 0)
	{
	  DBGLOG(DBG, "NewClient::handle_read_header(): Done for this round.");
	}
      else
	{
	  assert (header->compare(HEADER_ANS) == 0);

	  BackwardMessagePtr result(new BackwardMessage);

	  conn->async_read(*result,
			   boost::bind(&NewClient::handle_read_answer, this,
				       boost::asio::placeholders::error,
				       conn,
				       header,
				       mess,
				       result));	  
	}
    }
  else
    {
      DBGLOG(ERROR, "NewClient::handle_read_header(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }  
}



void
NewClient::handle_read_answer(const boost::system::error_code& e,
			      connection_ptr conn,
			      boost::shared_ptr<std::string> header,
			      ForwardMessagePtr mess,
			      BackwardMessagePtr result)
{
  if (!e)
    {
      ReturnedBeliefStateListPtr result_list = result->rbsl;
      
      ReturnedBeliefState* last_result = result_list->back();
      assert (last_result->belief_state == NULL);
      result_list->pop_back();
      delete last_result;
      last_result = 0;

      if (result_list->size() == 0)
	{
	  DBGLOG(DBG, "NewClient::handle_read_answer(): No more results. Let's get back.");
	}
      else
	{
	  std::size_t pack_size = mess->getPackSize();
	  std::size_t next_k = 0;

	  DBGLOG(DBG, "result_list.size() = " << result_list->size());
	  DBGLOG(DBG, "pack_size = " << pack_size);

	  if (pack_size > 0)
	    {
	      assert (result_list->size() <= pack_size);
	      next_k = pack_size - result_list->size();
	    }

	  if (callback)
	    {
	      callback(result_list);
	    }

	  if (next_k == 0)
	    {
	      DBGLOG(DBG, "NewClient::handle_read_answer(): Done. Let's get back.");
	    }
	  else
	    {
	      DBGLOG(DBG, "NewClient::handle_read_answer(): next_k = " << next_k);
	      std::size_t k_one = mess->k2 + 1;
	      std::size_t k_two = mess->k2 + next_k;

	      DBGLOG(DBG, "NewClient::handle_read_answer(): Issue another request = " << *mess);

	      mess->setPackRequest(k_one, k_two);

	      *header = HEADER_NEXT;

	      boost::system::error_code e;
	      boost::asio::ip::tcp::resolver::iterator it;
	      send_header(e, it, conn, header, mess);
	    }
	}

    }
  else
    {
      DBGLOG(ERROR, "NewClient::handle_read_answer(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }
}



void
NewClient::next(ForwardMessage& fwd_mess)
{
  boost::shared_ptr<std::string> header(new std::string(HEADER_NEXT));
  ForwardMessagePtr mess(new ForwardMessage(fwd_mess));

  boost::system::error_code e;
  boost::asio::ip::tcp::resolver::iterator it;
  send_header(e, it, conn, header, mess);
}


void
NewClient::terminate(ForwardMessage& end_mess)
{
  boost::shared_ptr<std::string> header(new std::string(HEADER_TERMINATE));
  ForwardMessagePtr mess(new ForwardMessage(end_mess));

  conn->async_write(*header,
		    boost::bind(&NewClient::handle_finalize, this,
				boost::asio::placeholders::error,
				conn,
				header,
				mess));
}



void
NewClient::handle_finalize(const boost::system::error_code& e,
			   connection_ptr conn,
			   boost::shared_ptr<std::string> header,
			   ForwardMessagePtr mess)
{
  if (!e)
    {
      conn->async_write(*mess,
			boost::bind(&NewClient::closing, this,
				    boost::asio::placeholders::error,
				    conn,
				    header));   
    }
  else
    {
      DBGLOG(ERROR, "NewClient::handle_finalize(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }
}



void
NewClient::closing(const boost::system::error_code& e,
		   connection_ptr conn,
		   boost::shared_ptr<std::string> /* header */)
{
  if (!e)
    {
      conn->socket().close();
    }
  else
    {
      DBGLOG(DBG, "NewClient::closing(): ERROR: " << e.message());
      throw std::runtime_error(e.message());
    }
}


} // namespace dmcs

// Local Variables:
// mode: C++
// End:
