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
 * @file   AsynClient.tcc
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  10 14:22:21 2011
 * 
 * @brief  
 * 
 * 
 */

#ifndef ASYN_CLIENT_TCC
#define ASYN_CLIENT_TCC

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "mcs/Theory.h"
#include "dmcs/StreamingBackwardMessage.h"
#include "dmcs/StreamingForwardMessage.h"

#include "dmcs/Log.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/map.hpp>

namespace dmcs {

template<typename ForwardMessType, typename BackwardMessType>
AsynClient<ForwardMessType, BackwardMessType>::AsynClient(boost::asio::io_service& ios,
							  boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
							  const std::string& h,
							  ForwardMessType& fm)
  : the_conn(new connection(ios)),
    callback(0)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);
  
  assert(fm.getPackSize() > 0);

  boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;

  boost::shared_ptr<std::string> header(new std::string(h));

  boost::shared_ptr<ForwardMessType> mess(new ForwardMessType(fm));

  the_conn->socket().async_connect(endpoint,
				   boost::bind(&AsynClient::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator,
					       the_conn,
					       header,
					       mess
					       )
				   );
}


template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::send_header(const boost::system::error_code& e,
							   boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
							   connection_ptr conn,
							   boost::shared_ptr<std::string> my_header,
							   boost::shared_ptr<ForwardMessType> mess)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_TRACE("Send header: " << *my_header << ", pass mess: " << *mess);

      // The connection is now established successfully
      conn->async_write(*my_header,
			boost::bind(&AsynClient::send_message, this,
				    boost::asio::placeholders::error,
				    conn,
				    my_header,
				    mess
				    )
			);
      
    }
  else if (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
    {
      // Try the next endpoint.
      conn->socket().close();
      
      boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
      conn->socket().async_connect(endpoint,
				   boost::bind(&AsynClient::send_header, this,
					       boost::asio::placeholders::error,
					       ++endpoint_iterator,
					       conn,
					       my_header,
					       mess
					       )
				   );
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::send_message(const boost::system::error_code& e,
							    connection_ptr conn,
							    boost::shared_ptr<std::string> my_header,
							    boost::shared_ptr<ForwardMessType> mess)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_DEBUG("Sending message: " << *mess << " for header: " << *my_header);

      conn->async_write(*mess,
			boost::bind(&AsynClient::read_header, this,
				    boost::asio::placeholders::error,
				    conn,
				    my_header,
				    mess
				    )
			);
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::read_header(const boost::system::error_code& e,
							   connection_ptr conn,
							   boost::shared_ptr<std::string> my_header,
							   boost::shared_ptr<ForwardMessType> mess)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      conn->async_read(*my_header,
		       boost::bind(&AsynClient::handle_read_header, this,
				   boost::asio::placeholders::error,
				   conn,
				   my_header,
				   mess
				   )
		       );
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::handle_read_header(const boost::system::error_code& e, 
								  connection_ptr conn,
								  boost::shared_ptr<std::string> my_header,
								  boost::shared_ptr<ForwardMessType> mess)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      DMCS_LOG_TRACE("Received header = " << *my_header);

      if (my_header->compare(HEADER_EOF) == 0) // read next or terminate
	{
	  // check for next thing to do here, calling function decides...
	  //
	  // we just run out of things to do for now

	  DMCS_LOG_TRACE("that's it for now");
	}
      else // read answer
	{
	  boost::shared_ptr<BackwardMessType> result(new BackwardMessType);

	  conn->async_read(*result,
			   boost::bind(&AsynClient::handle_read_answer, this,
				       boost::asio::placeholders::error,
				       conn,
				       my_header,
				       mess,
				       result
				       )
			   );

	}
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
void 
AsynClient<ForwardMessType, BackwardMessType>::handle_read_answer(const boost::system::error_code& e,
								  connection_ptr conn,
								  boost::shared_ptr<std::string> my_header,
								  boost::shared_ptr<ForwardMessType> mess,
								  boost::shared_ptr<BackwardMessType> result)

{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      const ModelSessionIdListPtr& msl = result->getResult();

      assert ( msl->size() <= mess->getPackSize() );

      const std::size_t next_k = mess->getPackSize() - msl->size();

      if (callback)
	{
	  callback(*result);
	}

      DMCS_LOG_TRACE("pack_size = " << mess->getPackSize() << ", msl->size() = " << msl->size() << ", next_k = " << next_k);

      if (next_k == 0)
	{
	  // done, fade out
	  DMCS_LOG_TRACE("done, let's get back");
	}
      else
	{
	  mess->setK1(mess->getK1() + msl->size());
	  DMCS_LOG_TRACE("requesting next batch: " << next_k << " from " << mess->getK1() << " to " << mess->getK2());

	  *my_header = HEADER_NEXT;

	  boost::system::error_code e;
	  boost::asio::ip::tcp::resolver::iterator it;

	  send_header(e, it, conn, my_header, mess);
	}
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}



template<typename ForwardMessType, typename BackwardMessType>
bool
AsynClient<ForwardMessType, BackwardMessType>::next(ForwardMessType& m)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  assert (m.getPackSize() > 0);

  // setup next k messages...

  boost::shared_ptr<std::string> my_header(new std::string(HEADER_NEXT));
  boost::shared_ptr<ForwardMessType> mess(new ForwardMessType(m));

  boost::system::error_code e;
  boost::asio::ip::tcp::resolver::iterator it;

  send_header(e, it, the_conn, my_header, mess);

  return true;
}



template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::terminate()
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  boost::shared_ptr<std::string> my_header(new std::string(HEADER_TERMINATE));

  the_conn->async_write(*my_header,
			boost::bind(&AsynClient::finalize, this,
				    boost::asio::placeholders::error,
				    the_conn,
				    my_header)
			);
}




template<typename ForwardMessType, typename BackwardMessType>
void
AsynClient<ForwardMessType, BackwardMessType>::finalize(const boost::system::error_code& e,
							connection_ptr /* conn */,
							boost::shared_ptr<std::string> /* my_header */)
{
  DMCS_LOG_DEBUG(__PRETTY_FUNCTION__);

  if (!e)
    {
      // Do nothing. Since we are not starting a new operation the
      // io_service will run out of work to do and the client will
      // exit.

      the_conn->socket().close();
    }
  else
    {
      // An error occurred.
      DMCS_LOG_ERROR(__PRETTY_FUNCTION__ << ": " << e.message());
      throw std::runtime_error(e.message());
    }
}


} // namespace dmcs

#endif // ASYN_CLIENT_TCC

// Local Variables:
// mode: C++
// End:
