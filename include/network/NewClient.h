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
 * @file   NewClient.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Mar  20 15:40:06 2012
 * 
 * @brief  
 * 
 * 
 */

#ifndef NEW_CLIENT_H
#define NEW_CLIENT_H

#include "mcs/ForwardMessage.h"
#include "mcs/BackwardMessage.h"
#include "network/connection.hpp"

namespace dmcs {

class NewClient
{
public:
  NewClient(boost::asio::io_service& io_service,
	    boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	    const std::string& h,
	    ForwardMessage& fwd_mess,
	    bool loop);

  typedef void (*CallbackFunc)(ReturnedBeliefStateListPtr);

  void
  setCallback(CallbackFunc f);

  void
  next(ForwardMessage& fwd_mess);

  void
  terminate(ForwardMessage& end_mess);

private:
  void
  send_header(const boost::system::error_code& e,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	      connection_ptr conn,
	      boost::shared_ptr<std::string> my_header,
	      ForwardMessagePtr mess);
  
  void
  send_message(const boost::system::error_code& e,
	       connection_ptr conn,
	       boost::shared_ptr<std::string> my_header,
	       ForwardMessagePtr mess);

  void
  read_header(const boost::system::error_code& e,
	      connection_ptr conn,
	      boost::shared_ptr<std::string> header,
	      ForwardMessagePtr mess);

  void 
  handle_read_header(const boost::system::error_code& e, 
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header,
		     ForwardMessagePtr mess);

  void
  handle_read_answer(const boost::system::error_code& e,
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header,
		     ForwardMessagePtr mess,
		     BackwardMessagePtr result);

  void
  handle_finalize(const boost::system::error_code& e,
		  connection_ptr conn,
		  boost::shared_ptr<std::string> header,
		  ForwardMessagePtr mess);

  void
  closing(const boost::system::error_code& e,
	  connection_ptr conn,
	  boost::shared_ptr<std::string> /* header */);

private:
  connection_ptr conn;
  CallbackFunc callback;
  bool loop;
};

} // namespace dmcs

#endif // NEW_CLIENT_H

// Local Variables:
// mode: C++
// End:
