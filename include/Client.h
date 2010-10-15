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
 * @file   Client.h
 * @author Thomas Krennwallner <tkren@kr.tuwien.ac.at>
 * @date   Sun Nov  8 10:44:51 2009
 * 
 * @brief  
 * 
 * 
 */



#ifndef CLIENT_H
#define CLIENT_H

#include "connection.hpp"
#include "BeliefState.h"
#include "CommandType.h"
#include "Message.h"
#include "Theory.h"

#include <boost/asio.hpp>

namespace dmcs {

/**
 * @brief
 */
template <typename CmdType>
class Client
{
 public:
  Client(boost::asio::io_service& io_service,
	 boost::asio::ip::tcp::resolver::iterator endpoint_iterator,
	 const std::string& header_, typename CmdType::input_type& mess);

  typename CmdType::return_type
  getResult()
  {
    return result;
  }

 private:
  void 
  send_header(const boost::system::error_code& error,
	      boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

  void
  send_message(const boost::system::error_code& error, connection_ptr conn);

  void
  read_header(const boost::system::error_code& error, connection_ptr conn);

  void 
  handle_read_header(const boost::system::error_code& error, connection_ptr conn);

  void 
  read_answer(const boost::system::error_code& error, connection_ptr conn);

  void 
  finalize(const boost::system::error_code& error, connection_ptr /* conn */);


  boost::asio::io_service& io_service_;

  const std::string& my_header; 
  std::string received_header;
  connection_ptr conn;
  typename CmdType::input_type mess;
  typename CmdType::return_type result;
};

} // namespace dmcs

#include "Client.tcc"

#endif

// Local Variables:
// mode: C++
// End:
