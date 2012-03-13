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
 * @file   DmcsClientTest.h
 * @author Minh Dao-Tran <dao@kr.tuwien.ac.at>
 * @date   Tue Mar  13 9:55:26 2012
 * 
 * @brief  
 * 
 * 
 */


#ifndef DMCSC_TEST_H
#define DMCSC_TEST_H

#include "mcs/ForwardMessage.h"
#include "mcs/BackwardMessage.h"
#include "network/connection.hpp"

class DmcsClientTest
{
public:
  DmcsClientTest(boost::asio::io_service& i,
		 boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
		 dmcs::ForwardMessage* ws1,
		 dmcs::ForwardMessage* ws2);

  void
  send_header(const boost::system::error_code& e,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	      connection_ptr conn,
	      dmcs::ForwardMessage* ws1,
	      dmcs::ForwardMessage* ws2);

  void
  send_message(const boost::system::error_code& e,
	       connection_ptr conn,
	       dmcs::ForwardMessage* ws1,
	       dmcs::ForwardMessage* ws2);

  void
  read_header(const boost::system::error_code& e,
	      connection_ptr conn,
	      dmcs::ForwardMessage* ws1,
	      dmcs::ForwardMessage* ws2);

  void
  handle_read_header(const boost::system::error_code& e,
		     connection_ptr conn,
		     boost::shared_ptr<std::string> header,
		     dmcs::ForwardMessage* ws1,
		     dmcs::ForwardMessage* ws2);

  void
  handle_read_message(const boost::system::error_code& e,
		      connection_ptr conn,
		      dmcs::BackwardMessagePtr bmess,
		      dmcs::ForwardMessage* ws1,
		      dmcs::ForwardMessage* ws2);

private:
  boost::asio::io_service& io_service;
  connection_ptr conn;
  std::size_t count;
};

#endif // DMCSC_TEST_H

// Local Variables:
// mode: C++
// End:
