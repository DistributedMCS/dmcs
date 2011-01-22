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
 * @file   BaseClient.h
 * @author Minh Dao Tran <dao@kr.tuwien.ac.at>
 * @date   Mon Jan  10 11:44:51 2011
 * 
 * @brief  
 * 
 * 
 */



#ifndef BASE_CLIENT_H
#define BASE_CLIENT_H

#include "network/connection.hpp"
#include <boost/asio.hpp>

namespace dmcs {

/**
 * @brief
 */
class BaseClient
{
public:
  BaseClient(boost::asio::io_service& io_service,
	     boost::asio::ip::tcp::resolver::iterator /* endpoint_iterator */,
	     const std::string& my_header_)
    : io_service_(io_service),
      conn(new connection(io_service)),
      my_header(my_header_)
  { }

protected:
  virtual void 
  send_header(const boost::system::error_code& error,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator) = 0;

  virtual void
  send_message(const boost::system::error_code& error, connection_ptr conn) = 0;

  virtual void
  read_header(const boost::system::error_code& error, connection_ptr conn) = 0;

  virtual void 
  handle_read_header(const boost::system::error_code& error, connection_ptr conn) = 0;

  virtual void 
  read_answer(const boost::system::error_code& error, connection_ptr conn) = 0;

  virtual void 
  finalize(const boost::system::error_code& error, connection_ptr /* conn */) = 0;

protected:
  boost::asio::io_service& io_service_;
  connection_ptr conn;
  const std::string my_header;
};

} // namespace dmcs

#endif // BASE_CLIENT_H

// Local Variables:
// mode: C++
// End:
